#!/usr/bin/env python3
"""
Generates compliance.md showing per-header libcxx conformance.

For each header:
  - Conformance: run uncached libcxx tests (up to N_SAMPLE new per run)
Conformance is shown as x/y/z/w where:
  x = psychicstd passes
  y = system STL passes (from all tests run so far)
  z = total tests run so far
  w = total eligible tests in the LLVM suite

Cache stores individual per-test results; incremental runs only run uncached tests.

Usage:
  tools/compliance.py                     # check all headers (up to N_SAMPLE new tests each)
  tools/compliance.py map                 # re-check only map
  tools/compliance.py --sample 50 vector  # run up to 50 new tests for vector
"""

import argparse
import json
import os
import random
import re
import shlex
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent.resolve()
PSYCHICSTD = REPO_ROOT / "include"
RUNTIME_SOURCES = sorted((REPO_ROOT / "src").glob("*.cpp"))
LLVM_ROOT = Path(
    os.environ.get("LLVM_ROOT", Path.home() / "code/thirdparty/llvm-project")
)
LIBCXX_TEST = LLVM_ROOT / "libcxx" / "test"
SUPPORT_DIR = LIBCXX_TEST / "support"

N_SAMPLE = 15  # new tests to add per header per run (default)
N_WORKERS = os.cpu_count() or 1
SEED = 42
LANGUAGE_STANDARD = 20

CXX = os.environ.get("CXX", "c++")
CXX_CMD = shlex.split(CXX)
AR = os.environ.get("AR", "ar")
RUNTIME_ARCHIVE: Path | None = None
_RUNTIME_DIRECTORY: tempfile.TemporaryDirectory | None = None

# Sanitizer mode: compile both STLs with ASan+UBSan and run the .pass tests, so
# runtime memory/UB bugs in psychicstd surface as failures the system STL does
# not have. -fno-sanitize-recover makes UBSan abort (it otherwise continues and
# the process exits 0). Uses a separate cache and output file.
SANITIZE = False
SAN_CFLAGS = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-g"]
SAN_ENV = {
    "ASAN_OPTIONS": "abort_on_error=1:detect_leaks=1",
    "UBSAN_OPTIONS": "print_stacktrace=1:halt_on_error=1",
}
# Extra compile flags (both STLs), e.g. -O2 for the nightly full run so slow
# tests fit in the per-test timeout. Per-test compile+run timeout in seconds.
EXTRA_CFLAGS: list[str] = []
TEST_TIMEOUT = 20
# The one-time archive build can contend with other jobs on a busy CI runner.
RUNTIME_BUILD_TIMEOUT = 120

# Known sanitizer failures; the gate fails only on failures NOT listed here.
BASELINE_FILE = REPO_ROOT / "tools" / "compliance_sanitize_baseline.txt"

HEADER_TO_DIRS: dict[str, list[str]] = {
    "algorithm": ["algorithms"],
    "any": ["utilities/any"],
    "array": ["containers/sequences/array"],
    "atomic": ["atomics"],
    "bit": ["numerics/bit"],
    "bitset": ["utilities/template.bitset"],
    "cassert": ["language.support/support.runtime"],
    "cctype": ["strings/c.strings"],
    "cerrno": ["depr/depr.c.headers"],
    "cfenv": ["numerics/cfenv"],
    "cfloat": ["depr/depr.c.headers"],
    "charconv": ["utilities/charconv"],
    "cinttypes": ["input.output/file.streams/c.files"],
    "clocale": ["depr/depr.c.headers"],
    "chrono": ["time"],
    "ciso646": ["depr/depr.c.headers"],
    "climits": ["language.support/support.limits"],
    "cmath": ["numerics/c.math"],
    "compare": ["language.support/cmp"],
    "complex": ["numerics/complex.number"],
    "concepts": ["concepts"],
    "condition_variable": ["thread/thread.condition"],
    "coroutine": ["language.support/support.coroutines"],
    "csetjmp": ["language.support/support.runtime"],
    "csignal": ["depr/depr.c.headers"],
    "cstdarg": ["language.support/support.runtime"],
    "cstddef": ["language.support/support.types"],
    "cstdint": ["utilities/intseq"],
    "cstdio": ["depr/depr.c.headers"],
    "cstdlib": ["language.support/support.runtime"],
    "cstring": ["strings/c.strings"],
    "ctime": ["depr/depr.c.headers"],
    "cwchar": ["depr/depr.c.headers"],
    "cwctype": ["strings/c.strings"],
    "deque": ["containers/sequences/deque"],
    "exception": ["language.support/support.exception"],
    "filesystem": ["input.output/filesystems"],
    "forward_list": ["containers/sequences/forwardlist"],
    "fstream": ["input.output/file.streams"],
    "functional": ["utilities/function.objects"],
    "future": ["thread/futures"],
    "initializer_list": ["language.support/support.initlist"],
    "iomanip": ["input.output/iostream.format"],
    "iostream": ["input.output/iostream.objects"],
    "ios": ["input.output/iostreams.base"],
    "iosfwd": ["input.output/iostream.forward"],
    "istream": ["input.output/iostream.format/input.streams"],
    "iterator": ["iterators"],
    "limits": ["language.support/support.limits/limits"],
    "list": ["containers/sequences/list"],
    "locale": ["localization"],
    "map": ["containers/associative/map"],
    "memory": ["utilities/memory"],
    "mutex": ["thread/thread.mutex"],
    "new": ["language.support/support.dynamic"],
    "numeric": ["numerics/numeric.ops"],
    "optional": ["utilities/optional"],
    "ostream": ["input.output/iostream.format/output.streams"],
    "queue": [
        "containers/container.adaptors/queue",
        "containers/container.adaptors/priority.queue",
    ],
    "random": ["numerics/rand"],
    "ranges": ["ranges"],
    "ratio": ["utilities/ratio"],
    "regex": ["re"],
    "scoped_allocator": ["utilities/allocator.adaptor"],
    "set": ["containers/associative/set"],
    "shared_mutex": [
        "thread/thread.mutex/thread.lock/thread.lock.shared",
        "thread/thread.mutex/thread.mutex.requirements/thread.shared_mutex.requirements",
    ],
    "source_location": ["language.support/support.srcloc"],
    "span": ["containers/views/views.span"],
    "sstream": ["input.output/string.streams"],
    "stack": ["containers/container.adaptors/stack"],
    "stdexcept": ["diagnostics/std.exceptions"],
    "stop_token": ["thread/thread.stoptoken"],
    "streambuf": ["input.output/stream.buffers"],
    "string": ["strings/basic.string"],
    "string_view": ["strings/string.view"],
    "system_error": ["diagnostics/syserr"],
    "thread": ["thread"],
    "tuple": ["utilities/tuple"],
    "typeinfo": ["language.support/support.rtti"],
    "type_traits": ["utilities/meta"],
    "typeindex": ["utilities/type.index"],
    "unordered_map": ["containers/unord/unord.map"],
    "unordered_set": ["containers/unord/unord.set"],
    "utility": ["utilities/utility"],
    "valarray": ["numerics/numarray"],
    "variant": ["utilities/variant"],
    "vector": ["containers/sequences/vector"],
    "version": ["language.support/support.limits/support.limits.general"],
}

EXCLUDED_PUBLIC_HEADERS = {"cxxabi.h", "uchar.h"}

_RE_UNSUPPORTED = re.compile(r"^//\s*UNSUPPORTED:\s*(.+)$", re.MULTILINE)
_RE_REQUIRES = re.compile(r"^//\s*REQUIRES:\s*(.+)$", re.MULTILINE)
_RE_ADD_FLAGS = re.compile(r"^//\s*ADDITIONAL_COMPILE_FLAGS:\s*(.+)$", re.MULTILINE)
_RE_EXACT_STANDARD = re.compile(r"(?<![\w-])c\+\+(03|11|14|17|20|23|26)(?![\w-])")
_RE_MINIMUM_STANDARD = re.compile(r"std-at-least-c\+\+(03|11|14|17|20|23|26)")


def _tokens(line: str) -> list[str]:
    return [t.strip() for t in re.split(r"[,\s]+", line) if t.strip()]


def _requires_other_standard(expression: str) -> bool:
    exact_standards = {int(value) for value in _RE_EXACT_STANDARD.findall(expression)}
    if exact_standards and LANGUAGE_STANDARD not in exact_standards:
        return True
    return any(
        int(value) > LANGUAGE_STANDARD
        for value in _RE_MINIMUM_STANDARD.findall(expression)
    )


def classify_test(path: Path) -> str:
    """Return eligible, libcpp-specific, or irrelevant for this test driver."""
    stem = path.stem
    if stem.endswith((".verify", ".fail")):
        return "irrelevant"
    if path.suffix != ".cpp":
        return "irrelevant"
    if not (stem.endswith((".pass", ".compile.pass"))):
        return "irrelevant"
    text = path.read_text(errors="replace")
    for m in _RE_UNSUPPORTED.finditer(text):
        # The report targets the supported C++20 baseline. Keep tests requiring
        # a newer language mode in the same ignored bucket as libc++-specific
        # tests rather than counting their expected failures against either STL.
        if "c++20" in _tokens(m.group(1)):
            return "libcpp-specific"
    for m in _RE_REQUIRES.finditer(text):
        if _requires_other_standard(m.group(1)):
            return "libcpp-specific"
        if any(t.startswith("libcpp-") for t in _tokens(m.group(1))):
            return "libcpp-specific"
    return "eligible"


def should_skip(path: Path) -> bool:
    return classify_test(path) != "eligible"


def extra_flags(text: str) -> list[str]:
    flags = []
    for m in _RE_ADD_FLAGS.finditer(text):
        flags.extend(m.group(1).split())
    return flags


def run_cmd(
    cmd: list[str], timeout: int = 20, env: dict | None = None
) -> tuple[bool, str]:
    try:
        r = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
            env=env,
            check=False,
        )
        return r.returncode == 0, (r.stdout + r.stderr).strip()
    except (subprocess.TimeoutExpired, FileNotFoundError) as e:
        return False, str(e)


def build_runtime_archive(flags: list[str]) -> Path:
    """Build the compiled psychicstd component used by conformance tests."""
    global _RUNTIME_DIRECTORY
    _RUNTIME_DIRECTORY = tempfile.TemporaryDirectory(prefix="psychicstd-compliance-")
    build_dir = Path(_RUNTIME_DIRECTORY.name)
    objects = []
    compile_flags = [
        f"-std=c++{LANGUAGE_STANDARD}",
        "-nostdinc++",
        "-fvisibility=hidden",
        f"-I{PSYCHICSTD}",
        *flags,
    ]
    for source in RUNTIME_SOURCES:
        obj = build_dir / f"{source.stem}.o"
        ok, output = run_cmd(
            [*CXX_CMD, *compile_flags, "-c", str(source), "-o", str(obj)],
            timeout=RUNTIME_BUILD_TIMEOUT,
        )
        if not ok:
            sys.exit(f"failed to build psychicstd runtime ({source.name}):\n{output}")
        objects.append(obj)

    archive = build_dir / "libpsychicstd.a"
    ok, output = run_cmd(
        [AR, "rcs", str(archive), *(str(obj) for obj in objects)],
        timeout=RUNTIME_BUILD_TIMEOUT,
    )
    if not ok:
        sys.exit(f"failed to archive psychicstd runtime:\n{output}")
    return archive


def try_compile_run(
    src: Path,
    flags: list[str],
    xflags: list[str],
    link_inputs: list[str],
    run_exe: bool,
) -> str:
    """Compile, link and optionally run a test; return pass/rfail/cfail."""
    with tempfile.NamedTemporaryFile(suffix="", delete=False, dir="/tmp") as f:
        exe = f.name
    obj = f"{exe}.o"
    try:
        compile_cmd = [
            *CXX_CMD,
            f"-std=c++{LANGUAGE_STANDARD}",
            *flags,
            *xflags,
            f"-I{SUPPORT_DIR}",
            "-c",
            str(src),
            "-o",
            obj,
        ]
        try:
            r = subprocess.run(
                compile_cmd, capture_output=True, timeout=TEST_TIMEOUT, check=False
            )
        except (subprocess.TimeoutExpired, FileNotFoundError):
            return "cfail"
        if r.returncode != 0:
            return "cfail"

        link_cmd = [*CXX_CMD, *flags, *xflags, obj, *link_inputs, "-o", exe]
        try:
            r = subprocess.run(
                link_cmd, capture_output=True, timeout=TEST_TIMEOUT, check=False
            )
        except (subprocess.TimeoutExpired, FileNotFoundError):
            return "cfail"
        if r.returncode != 0:
            return "cfail"
        if not run_exe:
            return "pass"
        env = {**os.environ, **SAN_ENV} if SANITIZE else None
        ok, _ = run_cmd([exe], timeout=TEST_TIMEOUT, env=env)
        return "pass" if ok else "rfail"
    finally:
        try:
            os.unlink(exe)
        except OSError:
            pass
        try:
            os.unlink(obj)
        except OSError:
            pass


def collect_tests(dirs: list[str]) -> tuple[list[Path], int]:
    tests: list[Path] = []
    for d in dirs:
        base = LIBCXX_TEST / "std" / d
        if base.exists():
            tests.extend(base.rglob("*.cpp"))
    classified = [(path, classify_test(path)) for path in set(tests)]
    eligible = sorted(path for path, kind in classified if kind == "eligible")
    ignored = sum(kind == "libcpp-specific" for _, kind in classified)
    return eligible, ignored


def collect_eligible(dirs: list[str]) -> list[Path]:
    return collect_tests(dirs)[0]


def compliance_emoji(n_pass: int, tested: int, relevant: int) -> str:
    if relevant == 0 or tested == 0:
        return "\u2b1c"
    pass_rate = n_pass / relevant
    if pass_rate >= 0.8:
        return "\U0001f7e2"
    if pass_rate < 0.2:
        return "\U0001f534"
    return "\U0001f7e1"


CACHE_FILE = REPO_ROOT / ".compliance_cache.json"
# Required keys in each cache entry (per-test format).
_CACHE_KEYS = frozenset({"tests", "lines", "eligible", "standard"})


def load_cache() -> dict:
    if not CACHE_FILE.exists():
        return {}
    try:
        data = json.loads(CACHE_FILE.read_text())
        # Reject the old aggregate-only format and discard obsolete timing data.
        return {
            header: {
                "tests": {
                    path: {"sys": test.get("sys"), "psy": test.get("psy")}
                    for path, test in value["tests"].items()
                },
                "lines": value["lines"],
                "eligible": value["eligible"],
                "ignored": value.get("ignored", 0),
                "standard": value["standard"],
            }
            for header, value in data.items()
            if _CACHE_KEYS.issubset(value)
        }
    except (json.JSONDecodeError, OSError):
        return {}


def save_cache(cache: dict) -> None:
    CACHE_FILE.write_text(json.dumps(cache, indent=2))


def test_id(path: str) -> str:
    """Stable, machine-independent id: path relative to libcxx/test."""
    return path.split("/libcxx/test/")[-1]


def load_baseline() -> set:
    if not BASELINE_FILE.exists():
        return set()
    return {
        s
        for line in BASELINE_FILE.read_text().splitlines()
        if (s := line.strip()) and not s.startswith("#")
    }


def save_baseline(ids: set) -> None:
    header = (
        "# Known sanitizer failures: libcxx tests that pass on libstdc++ but fail\n"
        "# under psychicstd+ASan/UBSan. The sanitizer CI fails only on failures NOT\n"
        "# listed here. Regenerate: tools/compliance.py --sanitize --update-baseline\n"
        "\n"
    )
    BASELINE_FILE.write_text(header + "\n".join(sorted(ids)) + "\n")


def header_exists(name: str) -> bool:
    return (PSYCHICSTD / name).exists()


def validate_public_headers() -> None:
    public = {
        path.name
        for path in PSYCHICSTD.iterdir()
        if path.is_file() and not path.name.startswith("__psychicstd_")
    }
    missing = public - HEADER_TO_DIRS.keys() - EXCLUDED_PUBLIC_HEADERS
    stale = EXCLUDED_PUBLIC_HEADERS - public
    if missing or stale:
        problems = []
        if missing:
            problems.append(f"unmapped public headers: {', '.join(sorted(missing))}")
        if stale:
            problems.append(
                f"stale public-header exclusions: {', '.join(sorted(stale))}"
            )
        sys.exit("; ".join(problems))


def header_lines(name: str) -> int:
    path = PSYCHICSTD / name
    try:
        return sum(1 for _ in path.open())
    except OSError:
        return 0


def libcxx_revision() -> str:
    try:
        return subprocess.run(
            ["git", "-C", str(LLVM_ROOT), "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            timeout=10,
            check=True,
        ).stdout.strip()
    except (OSError, subprocess.SubprocessError):
        return "unknown"


def _test_one(
    src: Path, sys_flags: list[str], psy_flags: list[str]
) -> tuple[str, dict]:
    """Compile/run one test with both sys and psy flags; return (path_str, entry)."""
    xf = extra_flags(src.read_text(errors="replace"))
    is_exec = not src.stem.endswith(".compile.pass")
    sys_status = try_compile_run(src, sys_flags, xf, [], is_exec)
    assert RUNTIME_ARCHIVE is not None
    psy_status = try_compile_run(src, psy_flags, xf, [str(RUNTIME_ARCHIVE)], is_exec)
    entry = {"sys": sys_status, "psy": psy_status}
    return str(src), entry


def _empty_header_cache() -> dict:
    return {
        "tests": {},
        "lines": 0,
        "eligible": 0,
        "ignored": 0,
        "standard": LANGUAGE_STANDARD,
    }


def _result_counts(tests: dict) -> dict[str, int]:
    counts = {"both_pass": 0, "system_only": 0, "psychic_only": 0, "both_fail": 0}
    for result in tests.values():
        system = result.get("sys") == "pass"
        psychic_status = result.get("psy")
        if psychic_status is None:
            continue
        psychic = psychic_status == "pass"
        if system and psychic:
            counts["both_pass"] += 1
        elif system:
            counts["system_only"] += 1
        elif psychic:
            counts["psychic_only"] += 1
        else:
            counts["both_fail"] += 1
    return counts


def _summary_from_cache(header: str, hc: dict) -> dict:
    """Build summary dict from cached data; no filesystem access."""
    tests: dict = hc.get("tests", {})
    counts = _result_counts(tests)
    tested = sum(counts.values())
    return {
        "header": header,
        "eligible": hc.get("eligible", 0),
        "ignored": hc.get("ignored", 0),
        "tested": tested,
        "psychic_passes": counts["both_pass"] + counts["psychic_only"],
        **counts,
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
    eligible, ignored = collect_tests(dirs)
    eligible_paths = {str(path) for path in eligible}

    cached_tests = {
        path: result
        for path, result in header_cache.get("tests", {}).items()
        if path in eligible_paths
    }

    san = SAN_CFLAGS if SANITIZE else []
    sys_flags: list[str] = [*san, *EXTRA_CFLAGS]
    # -fvisibility=hidden keeps psychicstd's inline symbols from interposing
    # libstdc++'s at runtime (which sanitizers/default libs pull in) -- the
    # documented way to use psychicstd; without it iostream init can crash.
    psy_flags = [
        "-nostdinc++",
        "-fvisibility=hidden",
        f"-I{PSYCHICSTD}",
        *san,
        *EXTRA_CFLAGS,
    ]

    if recheck:
        # Re-run previously cached tests; reset results so they run fresh
        to_run = sorted(Path(p) for p in cached_tests if Path(p).exists())
        cached_tests = {}
    else:
        # Also run old entries where psychicstd was skipped after a system failure.
        uncached = sorted(
            p
            for p in eligible
            if str(p) not in cached_tests or cached_tests[str(p)].get("psy") is None
        )
        rng = random.Random(SEED + len(cached_tests))
        to_run = rng.sample(uncached, min(n_sample, len(uncached)))
        to_run.sort()

    with ThreadPoolExecutor(max_workers=N_WORKERS) as pool:
        futs = [pool.submit(_test_one, src, sys_flags, psy_flags) for src in to_run]
        for fut in futs:
            path_str, entry = fut.result()
            cached_tests[path_str] = entry

    lines = header_lines(header)
    updated = {
        "tests": cached_tests,
        "lines": lines,
        "eligible": len(eligible),
        "ignored": ignored,
        "standard": LANGUAGE_STANDARD,
    }
    summary = _summary_from_cache(header, updated)
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
            f"  {CXX} -std=c++{LANGUAGE_STANDARD} -nostdinc++ "
            f"-I{PSYCHICSTD} -I{SUPPORT_DIR} "
            f"{example} /path/to/libpsychicstd.a -o /tmp/t && /tmp/t"
        )
    if not any_printed:
        print("No failing tests in cache.")


def main() -> None:
    global SANITIZE, CACHE_FILE, EXTRA_CFLAGS, TEST_TIMEOUT, RUNTIME_ARCHIVE
    global N_WORKERS
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
    ap.add_argument(
        "--sanitize",
        action="store_true",
        help="Compile+run tests under ASan/UBSan (both STLs) to catch runtime "
        "bugs; uses a separate cache and writes compliance.sanitize.md",
    )
    ap.add_argument(
        "--update-baseline",
        action="store_true",
        help="With --sanitize: write the current known failures to the baseline "
        "instead of failing on them",
    )
    ap.add_argument(
        "--cxxflags",
        default="",
        help="Extra compile flags for both STLs (e.g. '-O2' so slow tests fit "
        "the timeout in a full run)",
    )
    ap.add_argument(
        "--timeout",
        type=int,
        default=TEST_TIMEOUT,
        help=f"Per-test compile+run timeout in seconds (default: {TEST_TIMEOUT})",
    )
    ap.add_argument(
        "--jobs",
        type=int,
        default=N_WORKERS,
        metavar="N",
        help=f"Maximum parallel test jobs (default: {N_WORKERS})",
    )
    args = ap.parse_args()

    if args.jobs < 1:
        ap.error("--jobs must be at least 1")

    EXTRA_CFLAGS = args.cxxflags.split()
    TEST_TIMEOUT = args.timeout
    N_WORKERS = args.jobs

    if args.sanitize:
        SANITIZE = True
        CACHE_FILE = REPO_ROOT / ".compliance_cache.sanitize.json"

    validate_public_headers()
    all_headers = sorted(h for h in HEADER_TO_DIRS if header_exists(h))
    filter_set = set(args.headers)

    if filter_set:
        unknown = filter_set - set(HEADER_TO_DIRS)
        if unknown:
            print(f"Unknown headers: {', '.join(sorted(unknown))}", file=sys.stderr)
            sys.exit(1)

    cache = load_cache()
    revision = libcxx_revision()
    print(f"libc++ test revision: {revision}")

    if args.list_failing:
        headers_to_list = (
            sorted(filter_set & set(all_headers)) if filter_set else all_headers
        )
        _print_failing(headers_to_list, cache)
        return

    runtime_flags = [*(SAN_CFLAGS if SANITIZE else []), *EXTRA_CFLAGS]
    RUNTIME_ARCHIVE = build_runtime_archive(runtime_flags)

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
        n_pass = summary["psychic_passes"]
        tested = summary["tested"]
        eligible = summary["eligible"]
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
        print(f"  {n_pass}/{eligible} passed  {tested}/{eligible} tested{change_str}")

    save_cache(cache)

    # Rows for table: fresh results for headers we ran, cached for the rest
    rows = []
    for h in all_headers:
        if h in fresh_summaries:
            rows.append(fresh_summaries[h])
        elif h in cache:
            rows.append(_summary_from_cache(h, cache[h]))

    out = REPO_ROOT / ("compliance.sanitize.md" if SANITIZE else "compliance.md")
    with open(out, "w") as f:
        f.write("# Compliance\n\n")
        f.write(
            f"Last updated: {datetime.now().astimezone().strftime('%Y-%m-%d %H:%M')}\n\n"
        )
        f.write(f"libc++ test revision: `{revision}`\n\n")

        f.write("## Summary\n\n")
        f.write(
            "The pass percentage uses all tests except those explicitly marked "
            "inapplicable to the C++20 configuration. Untested cases are not "
            "counted as passes.\n\n"
        )
        f.write(
            "\U0001f7e2 at least 80% pass  "
            "\U0001f7e1 20% to 79% pass  "
            "\U0001f534 less than 20% pass  "
            "\u2b1c no relevant case has been tested\n\n"
        )

        f.write(
            "| | header | psychicstd passes | tested | relevant tests | "
            "ignored inapplicable tests | upstream total | lines |\n"
        )
        f.write(
            "|--|--------|------------------:|-------:|---------------:|"
            "------------------------:|---------------:|------:|\n"
        )

        for r in rows:
            header = r["header"]
            eligible = r["eligible"]
            tested = r["tested"]
            n_pass = r["psychic_passes"]
            ignored = r["ignored"]
            c_emoji = compliance_emoji(n_pass, tested, eligible)
            percentage = f"{100 * n_pass / eligible:.0f}%" if eligible else "n/a"

            f.write(
                f"| {c_emoji} | `{header}` | **{n_pass}/{eligible} "
                f"({percentage})** | {tested}/{eligible} | {eligible} | {ignored} | "
                f"{eligible + ignored} | {r['lines']} |\n"
            )

        f.write("\n## Library comparison\n\n")
        f.write(
            "Each tested case appears in exactly one of the first four result "
            "columns. Untested means relevant tests that have not yet been "
            "selected by incremental sampling. It does not include inapplicable "
            "tests; those are excluded from the relevant corpus and counted "
            "separately in the summary table.\n\n"
        )
        f.write(
            "| header | both pass | libstdc++ only | psychicstd only | both fail | "
            "untested |\n"
        )
        f.write(
            "|--------|----------:|----------------:|----------------:|----------:|---------:|\n"
        )
        for r in rows:
            f.write(
                f"| `{r['header']}` | {r['both_pass']} | {r['system_only']} | "
                f"{r['psychic_only']} | {r['both_fail']} | "
                f"{r['eligible'] - r['tested']} |\n"
            )

    subprocess.run(["mdformat", out], check=True)
    print(f"\nWrote {out}")

    # In sanitizer mode a runtime failure the system STL does not have is a
    # psychicstd bug. Gate against a committed baseline: only failures NOT already
    # known (regressions) turn CI red.
    if SANITIZE:
        failing = {
            test_id(path)
            for hc in cache.values()
            for path, e in hc.get("tests", {}).items()
            if e.get("sys") == "pass" and e.get("psy") == "rfail"
        }
        ran = {
            test_id(path)
            for hc in cache.values()
            for path, e in hc.get("tests", {}).items()
            if e.get("sys") == "pass" and e.get("psy") in ("pass", "rfail")
        }
        if args.update_baseline:
            save_baseline(failing)
            print(f"\nWrote {len(failing)} known failure(s) to {BASELINE_FILE.name}")
            return
        baseline = load_baseline()
        known = sorted(failing & baseline)
        fixed = sorted((baseline & ran) - failing)
        new = sorted(failing - baseline)
        if known:
            print(f"\n{len(known)} known sanitizer failure(s) (baselined):")
            for p in known:
                print(f"  {p}")
        if fixed:
            print(
                f"\n{len(fixed)} baselined failure(s) now PASS -- drop from baseline "
                "(--update-baseline):"
            )
            for p in fixed:
                print(f"  {p}")
        if new:
            print(f"\n{len(new)} NEW sanitizer failure(s) (regression):")
            for p in new:
                print(f"  {p}")
            sys.exit(1)
        print("\nNo new sanitizer failures.")


if __name__ == "__main__":
    main()
