#!/usr/bin/env python3
"""
Generates compliance.md showing per-header libcxx conformance and compile speed.

For each header:
  - Conformance: run uncached libcxx tests (up to N_SAMPLE new per run)
  - Speed: median compile time of one libcxx test file, psychicstd vs system

Conformance is shown as x/y/z/w where:
  x = psychicstd passes
  y = system STL passes (from all tests run so far)
  z = total tests run so far
  w = total eligible tests in the LLVM suite

Cache stores individual per-test results; incremental runs only run uncached tests.

Usage:
  python3 tools/compliance.py                     # check all headers (up to N_SAMPLE new tests each)
  python3 tools/compliance.py map                 # re-check only map
  python3 tools/compliance.py --sample 50 vector  # run up to 50 new tests for vector
"""

import argparse
import json
import os
import random
import re
import statistics
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent.resolve()
PSYCHICSTD = REPO_ROOT / "include"
LLVM_ROOT = Path(
    os.environ.get("LLVM_ROOT", Path.home() / "code/thirdparty/llvm-project")
)
LIBCXX_TEST = LLVM_ROOT / "libcxx" / "test"
SUPPORT_DIR = LIBCXX_TEST / "support"

N_SAMPLE = 15  # new tests to add per header per run (default)
N_BENCH = 1  # compile runs for timing
N_WORKERS = os.cpu_count() or 1
SEED = 42
SPEED_GREEN = 1.2
SPEED_RED = 0.8

CXX = os.environ.get("CXX", "g++")

HEADER_TO_DIRS: dict[str, list[str]] = {
    "algorithm": ["algorithms"],
    "array": ["containers/sequences/array"],
    "cassert": ["language.support/support.runtime"],
    "cctype": ["strings/c.strings"],
    "cerrno": ["depr/depr.c.headers"],
    "cfloat": ["depr/depr.c.headers"],
    "chrono": ["time"],
    "ciso646": ["depr/depr.c.headers"],
    "climits": ["language.support/support.limits"],
    "cmath": ["numerics/c.math"],
    "compare": ["language.support/cmp"],
    "complex": ["numerics/complex.number"],
    "concepts": ["concepts"],
    "cstddef": ["language.support/support.types"],
    "cstdint": ["utilities/intseq"],
    "cstdio": ["depr/depr.c.headers"],
    "cstdlib": ["language.support/support.runtime"],
    "cstring": ["strings/c.strings"],
    "ctime": ["depr/depr.c.headers"],
    "cwchar": ["depr/depr.c.headers"],
    "deque": ["containers/sequences/deque"],
    "exception": ["language.support/support.exception"],
    "fstream": ["input.output/file.streams"],
    "functional": ["utilities/function.objects"],
    "initializer_list": ["utilities/initializer.list"],
    "iomanip": ["input.output/iostream.format"],
    "iostream": ["input.output/iostream.objects"],
    "iterator": ["iterators"],
    "limits": ["language.support/support.limits/limits"],
    "list": ["containers/sequences/list"],
    "map": ["containers/associative/map"],
    "memory": ["utilities/memory"],
    "new": ["language.support/support.dynamic"],
    "numeric": ["numerics/numeric.ops"],
    "optional": ["utilities/optional"],
    "ostream": ["input.output/iostream.format/output.streams"],
    "random": ["numerics/rand"],
    "ranges": ["ranges"],
    "ratio": ["utilities/ratio"],
    "regex": ["re"],
    "set": ["containers/associative/set"],
    "sstream": ["input.output/string.streams"],
    "stack": ["containers/container.adaptors/stack"],
    "stdexcept": ["diagnostics/std.exceptions"],
    "streambuf": ["input.output/stream.buffers"],
    "string": ["strings/basic.string"],
    "string_view": ["strings/string.view"],
    "thread": ["thread"],
    "tuple": ["utilities/tuple"],
    "typeinfo": ["language.support/support.rtti"],
    "type_traits": ["utilities/meta"],
    "unordered_map": ["containers/unord/unord.map"],
    "unordered_set": ["containers/unord/unord.set"],
    "utility": ["utilities/utility"],
    "vector": ["containers/sequences/vector"],
}

_RE_UNSUPPORTED = re.compile(r"^//\s*UNSUPPORTED:\s*(.+)$", re.MULTILINE)
_RE_REQUIRES = re.compile(r"^//\s*REQUIRES:\s*(.+)$", re.MULTILINE)
_RE_ADD_FLAGS = re.compile(r"^//\s*ADDITIONAL_COMPILE_FLAGS:\s*(.+)$", re.MULTILINE)


def _tokens(line: str) -> list[str]:
    return [t.strip() for t in re.split(r"[,\s]+", line) if t.strip()]


def should_skip(path: Path) -> bool:
    stem = path.stem
    if stem.endswith(".verify") or stem.endswith(".fail"):
        return True
    if path.suffix != ".cpp":
        return True
    if not (stem.endswith(".pass") or stem.endswith(".compile.pass")):
        return True
    text = path.read_text(errors="replace")
    for m in _RE_UNSUPPORTED.finditer(text):
        if "c++23" in _tokens(m.group(1)):
            return True
    for m in _RE_REQUIRES.finditer(text):
        if any(t.startswith("libcpp-") for t in _tokens(m.group(1))):
            return True
    return False


def extra_flags(text: str) -> list[str]:
    flags = []
    for m in _RE_ADD_FLAGS.finditer(text):
        flags.extend(m.group(1).split())
    return flags


def run_cmd(cmd: list[str], timeout: int = 20) -> tuple[bool, str]:
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        return r.returncode == 0, (r.stdout + r.stderr).strip()
    except (subprocess.TimeoutExpired, FileNotFoundError) as e:
        return False, str(e)


def try_compile_run(
    src: Path, flags: list[str], xflags: list[str], run_exe: bool
) -> tuple[str, float | None]:
    """Returns ('pass'/'rfail'/'cfail', compile_ms or None if compile failed)."""
    with tempfile.NamedTemporaryFile(suffix="", delete=False, dir="/tmp") as f:
        exe = f.name
    try:
        cmd = [
            CXX,
            "-std=c++23",
            *flags,
            *xflags,
            f"-I{SUPPORT_DIR}",
            str(src),
            "-o",
            exe,
        ]
        t0 = time.perf_counter()
        try:
            r = subprocess.run(cmd, capture_output=True, timeout=20)
        except (subprocess.TimeoutExpired, FileNotFoundError):
            return "cfail", None
        compile_ms = (time.perf_counter() - t0) * 1000
        if r.returncode != 0:
            return "cfail", None
        if not run_exe:
            return "pass", compile_ms
        ok, _ = run_cmd([exe])
        return ("pass" if ok else "rfail"), compile_ms
    finally:
        try:
            os.unlink(exe)
        except OSError:
            pass


def collect_eligible(dirs: list[str]) -> list[Path]:
    tests: list[Path] = []
    for d in dirs:
        base = LIBCXX_TEST / "std" / d
        if base.exists():
            tests.extend(base.rglob("*.cpp"))
    return sorted(p for p in set(tests) if not should_skip(p))


def speed_emoji(ratio: float) -> str:
    if ratio > SPEED_GREEN:
        return "\U0001f7e2"
    if ratio >= SPEED_RED:
        return "\U0001f7e1"
    return "\U0001f534"


def compliance_emoji(n_pass: int, n_compile_ok: int, useful: int) -> str:
    if useful == 0:
        return "\u2b1c"
    if n_pass == useful:
        return "\U0001f7e2"
    if n_compile_ok > 0:
        return "\U0001f7e1"
    return "\U0001f534"


CACHE_FILE = REPO_ROOT / ".compliance_cache.json"
# Required keys in each cache entry (new per-test format)
_CACHE_KEYS = frozenset({"tests", "sys_ms", "psy_ms", "lines", "eligible"})


def load_cache() -> dict:
    if not CACHE_FILE.exists():
        return {}
    try:
        data = json.loads(CACHE_FILE.read_text())
        # Reject entries missing any required key (handles old aggregate-only format)
        return {k: v for k, v in data.items() if _CACHE_KEYS.issubset(v)}
    except (json.JSONDecodeError, OSError):
        return {}


def save_cache(cache: dict) -> None:
    CACHE_FILE.write_text(json.dumps(cache, indent=2))


def header_exists(name: str) -> bool:
    return (PSYCHICSTD / name).exists()


def header_lines(name: str) -> int:
    path = PSYCHICSTD / name
    try:
        return sum(1 for _ in path.open())
    except OSError:
        return 0


def _test_one(
    src: Path, sys_flags: list[str], psy_flags: list[str]
) -> tuple[str, dict]:
    """Compile/run one test with both sys and psy flags; return (path_str, entry)."""
    xf = extra_flags(src.read_text(errors="replace"))
    is_exec = not src.stem.endswith(".compile.pass")
    sys_status, sys_ms = try_compile_run(src, sys_flags, xf, is_exec)
    entry: dict = {"sys": sys_status, "sys_ms": sys_ms, "psy": None, "psy_ms": None}
    if sys_status == "pass":
        psy_status, psy_ms = try_compile_run(src, psy_flags, xf, is_exec)
        entry["psy"] = psy_status
        entry["psy_ms"] = psy_ms
    return str(src), entry


def _empty_header_cache() -> dict:
    return {
        "tests": {},
        "sys_ms": None,
        "psy_ms": None,
        "lines": 0,
        "eligible": 0,
    }


def _summary_from_cache(header: str, hc: dict) -> dict:
    """Build summary dict from cached data; no filesystem access."""
    tests: dict = hc.get("tests", {})
    n_pass = sum(1 for v in tests.values() if v.get("psy") == "pass")
    n_cfail = sum(1 for v in tests.values() if v.get("psy") == "cfail")
    n_rfail = sum(1 for v in tests.values() if v.get("psy") == "rfail")
    useful = n_pass + n_cfail + n_rfail
    return {
        "header": header,
        "eligible": hc.get("eligible", 0),
        "n_sampled": len(tests),
        "useful": useful,
        "n_pass": n_pass,
        "n_cfail": n_cfail,
        "n_rfail": n_rfail,
        "sys_ms": hc.get("sys_ms"),
        "psy_ms": hc.get("psy_ms"),
        "lines": hc.get("lines", 0),
    }


def check_header(
    header: str,
    dirs: list[str],
    n_sample: int,
    header_cache: dict,
    recheck: bool = False,
) -> tuple[dict, dict]:
    """
    Run tests for this header and return (updated_header_cache, summary_dict).

    Normal mode: run up to n_sample new (uncached) tests.
    recheck=True: re-run all previously cached tests to refresh their results.
    """
    eligible = collect_eligible(dirs)

    cached_tests: dict = dict(header_cache.get("tests", {}))

    sys_flags: list[str] = []
    psy_flags = ["-nostdinc++", f"-I{PSYCHICSTD}"]

    if recheck:
        # Re-run previously cached tests; reset results so they run fresh
        to_run = sorted(Path(p) for p in cached_tests if Path(p).exists())
        cached_tests = {}
    else:
        # Run up to n_sample tests not yet in the cache
        uncached = sorted(p for p in eligible if str(p) not in cached_tests)
        rng = random.Random(SEED + len(cached_tests))
        to_run = rng.sample(uncached, min(n_sample, len(uncached)))
        to_run.sort()

    with ThreadPoolExecutor(max_workers=N_WORKERS) as pool:
        futs = [pool.submit(_test_one, src, sys_flags, psy_flags) for src in to_run]
        for fut in futs:
            path_str, entry = fut.result()
            cached_tests[path_str] = entry

    # Derive header-level timing as median of per-test measurements
    sys_times = [
        v["sys_ms"] for v in cached_tests.values() if v.get("sys_ms") is not None
    ]
    psy_times = [
        v["psy_ms"] for v in cached_tests.values() if v.get("psy_ms") is not None
    ]
    sys_ms: float | None = statistics.median(sys_times) if sys_times else None
    psy_ms: float | None = statistics.median(psy_times) if psy_times else None

    n_pass = sum(1 for v in cached_tests.values() if v.get("psy") == "pass")
    n_cfail = sum(1 for v in cached_tests.values() if v.get("psy") == "cfail")
    n_rfail = sum(1 for v in cached_tests.values() if v.get("psy") == "rfail")
    useful = n_pass + n_cfail + n_rfail

    lines = header_lines(header)
    updated = {
        "tests": cached_tests,
        "sys_ms": sys_ms,
        "psy_ms": psy_ms,
        "lines": lines,
        "eligible": len(eligible),
    }
    summary = {
        "header": header,
        "eligible": len(eligible),
        "n_sampled": len(cached_tests),
        "useful": useful,
        "n_pass": n_pass,
        "n_cfail": n_cfail,
        "n_rfail": n_rfail,
        "sys_ms": sys_ms,
        "psy_ms": psy_ms,
        "lines": lines,
    }
    return updated, summary


def _print_failing(headers: list[str], cache: dict) -> None:
    """Print failing tests from the cache with a ready-to-use compiler command."""
    any_printed = False
    for h in headers:
        if h not in cache:
            continue
        tests: dict = cache[h].get("tests", {})
        failing = sorted(
            (p, v)
            for p, v in tests.items()
            if v.get("sys") == "pass" and v.get("psy") != "pass"
        )
        if not failing:
            continue
        any_printed = True
        print(f"\n{h}: {len(failing)} failing")
        for path_str, v in failing:
            print(f"  [{v.get('psy', '?')}] {path_str}")
        example = failing[0][0]
        print("\n  Quick test (psychicstd):")
        print(
            f"  g++ -std=c++23 -nostdinc++ -I{PSYCHICSTD} -I{SUPPORT_DIR} {example} -o /tmp/t && /tmp/t"
        )
    if not any_printed:
        print("No failing tests in cache.")


def main() -> None:
    ap = argparse.ArgumentParser(description="Generate compliance.md")
    ap.add_argument("headers", nargs="*", help="Headers to filter; default: all")
    ap.add_argument(
        "--sample",
        type=int,
        default=N_SAMPLE,
        metavar="N",
        help=f"Max new tests to run per header (default: {N_SAMPLE})",
    )
    ap.add_argument(
        "--recheck",
        action="store_true",
        help="Re-run all cached tests instead of only new ones; reports status changes",
    )
    ap.add_argument(
        "--list-failing",
        action="store_true",
        help="Print failing tests from cache (no compilation)",
    )
    args = ap.parse_args()

    all_headers = sorted(h for h in HEADER_TO_DIRS if header_exists(h))
    filter_set = set(args.headers)

    if filter_set:
        unknown = filter_set - set(HEADER_TO_DIRS)
        if unknown:
            print(f"Unknown headers: {', '.join(sorted(unknown))}", file=sys.stderr)
            sys.exit(1)

    cache = load_cache()

    if args.list_failing:
        headers_to_list = (
            sorted(filter_set & set(all_headers)) if filter_set else all_headers
        )
        _print_failing(headers_to_list, cache)
        return

    n_sample = args.sample
    to_run = (filter_set & set(all_headers)) if filter_set else set(all_headers)

    mode = "rechecking" if args.recheck else f"up to {n_sample} new tests each"
    print(f"Checking {len(to_run)} header(s) ({mode}) ...", flush=True)

    fresh_summaries: dict[str, dict] = {}
    for h in sorted(to_run):
        print(f"  {h:<16}", end="", flush=True)
        hc = cache.get(h, _empty_header_cache())
        old_tests = dict(hc.get("tests", {})) if args.recheck else {}
        updated, summary = check_header(
            h, HEADER_TO_DIRS[h], n_sample, hc, recheck=args.recheck
        )
        cache[h] = updated
        fresh_summaries[h] = summary
        n_pass = summary["n_pass"]
        useful = summary["useful"]
        n_sampled = summary["n_sampled"]
        eligible = summary["eligible"]
        speed_str = ""
        if summary["sys_ms"] and summary["psy_ms"]:
            ratio = summary["sys_ms"] / summary["psy_ms"]
            speed_str = f"  {ratio:.1f}x"
        change_str = ""
        if args.recheck and old_tests:
            new_tests = updated["tests"]
            gained = sum(
                1
                for p, v in new_tests.items()
                if v.get("psy") == "pass" and old_tests.get(p, {}).get("psy") != "pass"
            )
            lost = sum(
                1
                for p, v in new_tests.items()
                if v.get("psy") != "pass" and old_tests.get(p, {}).get("psy") == "pass"
            )
            if gained:
                change_str += f"  +{gained}"
            if lost:
                change_str += f"  -{lost}"
        print(f"  {n_pass}/{useful}/{n_sampled}/{eligible}{speed_str}{change_str}")

    save_cache(cache)

    # Rows for table: fresh results for headers we ran, cached for the rest
    rows = []
    for h in all_headers:
        if h in fresh_summaries:
            rows.append(fresh_summaries[h])
        elif h in cache:
            rows.append(_summary_from_cache(h, cache[h]))

    out = REPO_ROOT / "compliance.md"
    with open(out, "w") as f:
        f.write("# Compliance\n\n")
        f.write(f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M')}\n\n")

        f.write("## Conformance\n\n")
        f.write(
            "Conformance is shown as **x/y/z/w** where:\n\n"
            "- **x** — tests that pass with psychicstd\n"
            "- **y** — tests where the system STL (libstdc++) passes "
            "(only these are run against psychicstd; the rest are excluded as libc++-specific)\n"
            "- **z** — total tests run so far for this header (grows with each incremental run)\n"
            "- **w** — total eligible tests available for that header in the LLVM suite\n\n"
        )
        f.write(
            "\U0001f7e2 all sampled tests pass  "
            "\U0001f7e1 at least one test compiles (but not all pass)  "
            "\U0001f534 nothing compiles\n\n"
        )

        f.write("## Compilation speed\n\n")
        runs = "once" if N_BENCH == 1 else f"{N_BENCH} times (median reported)"
        f.write(
            f"One libcxx test file (the first that passes the system STL) is compiled {runs} "
            "with each STL. The speedup is the ratio of system time to psychicstd time — "
            "higher is better. n/a means no test file compiled successfully with the system "
            "STL in the sample, so no timing was available.\n\n"
        )
        f.write(
            f"\U0001f7e2 >{SPEED_GREEN}\u00d7  \U0001f7e1 {SPEED_RED}\u00d7\u2013{SPEED_GREEN}\u00d7  \U0001f534 <{SPEED_RED}\u00d7\n\n"
        )

        f.write("## Results\n\n")
        f.write("| | header | conformance | system | psychicstd | speedup | lines |\n")
        f.write("|--|--------|------------|-------:|----------:|--------:|------:|\n")

        for r in rows:
            header = r["header"]
            useful = r["useful"]
            n_pass = r["n_pass"]
            n_compile = n_pass + r["n_rfail"]

            c_emoji = compliance_emoji(n_pass, n_compile, useful)
            eligible = r["eligible"]
            n_sampled = r["n_sampled"]
            if useful or n_sampled:
                conform_cell = f"{c_emoji} {n_pass}/{useful}/{n_sampled}/{eligible}"
            else:
                conform_cell = f"\u2b1c 0/0/0/{eligible}"

            if r["sys_ms"] and r["psy_ms"]:
                ratio = r["sys_ms"] / r["psy_ms"]
                s_emoji = speed_emoji(ratio)
                sys_cell = f"{r['sys_ms']:.0f} ms"
                psy_cell = f"{r['psy_ms']:.0f} ms"
                spd_cell = f"{s_emoji} {ratio:.1f}\u00d7"
            else:
                sys_cell = "n/a"
                psy_cell = "n/a"
                spd_cell = "\u2b1c n/a"

            f.write(
                f"| {c_emoji} | `{header}` | {conform_cell} | {sys_cell} | {psy_cell} | {spd_cell} | {r['lines']} |\n"
            )

    subprocess.run(["mdformat", out], check=True)
    print(f"\nWrote {out}")


if __name__ == "__main__":
    main()
