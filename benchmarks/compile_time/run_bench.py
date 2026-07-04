#!/usr/bin/env python3
"""
Measures median compile time for each benchmark file, system STL vs psychicstd.
Writes results to stdout and updates speed.md in the repo root.
Usage: run_bench.py [cxx_compiler] [psychicstd_include_dir]
       [--extra-include name:path ...]
       [--bench-file path:name:include_key ...]
"""

import argparse
import json
import os
import statistics
import subprocess
import time
from datetime import datetime
from pathlib import Path

os.environ["CCACHE_DISABLE"] = "1"

BENCH_DIR = Path(__file__).parent.resolve()
REPO_ROOT = BENCH_DIR.parent.parent
N = int(os.environ.get("BENCH_N", "10"))  # reps per file; lower for quick runs

THRESHOLD_GREEN = 1.2  # above this: psychicstd is meaningfully faster
THRESHOLD_RED = 0.8  # below this: psychicstd is meaningfully slower

RED = "\033[31m"
YELLOW = "\033[33m"
GREEN = "\033[32m"
DIM = "\033[2m"
RESET = "\033[0m"


def compile_ms(cxx: str, flags: list[str], file: Path) -> float:
    cmd = [cxx, *flags, "-std=c++23", "-c", str(file), "-o", "/dev/null"]
    start = time.perf_counter()
    subprocess.run(cmd, check=True, stderr=subprocess.DEVNULL)
    return (time.perf_counter() - start) * 1000


def samples_ms(cxx: str, flags: list[str], file: Path) -> list[float] | None:
    """Returns the list of per-rep compile times in ms, or None if the file
    fails to compile with these flags. Keeping the raw samples (not just the
    median) lets the diff tool put a confidence interval on the change."""
    try:
        return [round(compile_ms(cxx, flags, file), 3) for _ in range(N)]
    except subprocess.CalledProcessError:
        return None


def median_ms(cxx: str, flags: list[str], file: Path) -> float | None:
    s = samples_ms(cxx, flags, file)
    return statistics.median(s) if s else None


def color(ratio: float) -> str:
    if ratio > THRESHOLD_GREEN:
        return GREEN
    if ratio >= THRESHOLD_RED:
        return YELLOW
    return RED


def emoji(ratio: float) -> str:
    if ratio > THRESHOLD_GREEN:
        return "🟢"
    if ratio >= THRESHOLD_RED:
        return "🟡"
    return "🔴"


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("compiler", nargs="?", default="g++")
    parser.add_argument("psychicstd_inc", nargs="?", default=str(REPO_ROOT / "include"))
    parser.add_argument(
        "--extra-include",
        action="append",
        default=[],
        metavar="name:path",
        help="Extra include path for a named key (e.g. rapidjson:/path/to/include)",
    )
    parser.add_argument(
        "--bench-file",
        action="append",
        default=[],
        metavar="path:name:include_key",
        help="Additional file to benchmark; include_key references an --extra-include name",
    )
    parser.add_argument(
        "--json",
        metavar="PATH",
        help="Write per-file results as JSON to PATH (for CI diffing) and skip speed.md",
    )
    args = parser.parse_args()

    # extra_includes maps name -> list of paths (colon-separated paths after name:)
    extra_includes: dict[str, list[Path]] = {}
    for spec in args.extra_include:
        name, _, paths_str = spec.partition(":")
        if name and paths_str:
            extra_includes[name] = [Path(p) for p in paths_str.split(":") if p]

    cxx = args.compiler
    psychicstd_flags = ["-nostdinc++", f"-I{args.psychicstd_inc}"]

    try:
        cxx_version = subprocess.run(
            [cxx, "--version"], capture_output=True, text=True
        ).stdout.splitlines()[0]
    except (OSError, IndexError):
        cxx_version = cxx

    # Auto-discover third_party/<name>/include dirs; CLI --extra-include takes precedence.
    third_party = BENCH_DIR / "third_party"
    if third_party.is_dir():
        for entry in third_party.iterdir():
            if entry.is_dir() and entry.name not in extra_includes:
                inc = entry / "include"
                if inc.is_dir():
                    extra_includes[entry.name] = [inc]

    # Auto-discover FetchContent deps in build/_deps/<name>-src.
    # Try common include subdirs; for catch2, also pick up generated-includes.
    deps_dir = REPO_ROOT / "build" / "_deps"
    if deps_dir.is_dir():
        for src_dir in deps_dir.glob("*-src"):
            name = src_dir.name[: -len("-src")]
            if name in extra_includes:
                continue
            paths = []
            for subdir in ("include", "src"):
                cand = src_dir / subdir
                if cand.is_dir():
                    paths.append(cand)
                    break
            gen = deps_dir / f"{name}-build" / "generated-includes"
            if gen.is_dir():
                paths.append(gen)
            if paths:
                extra_includes[name] = paths

    print(f"{DIM}compiler: {cxx_version}{RESET}")

    # Collect files to benchmark: local bench_*.cpp + any --bench-file additions
    # Each entry: (Path, display_name, include_key_or_empty)
    bench_files: list[tuple[Path, str, str]] = []

    # Auto-discover real-code fixture dirs (benchmarks/compile_time/<name>/*.cpp)
    # whose name matches a known extra_include key (e.g. rapidjson). These are
    # snippets of real third-party code used purely as compile-speed inputs.
    for group in sorted(BENCH_DIR.iterdir()):
        if not group.is_dir() or group.name == "third_party":
            continue
        key = group.name
        for cpp in sorted(group.glob("*.cpp")):
            label = f"{key}/{cpp.stem.removeprefix('test_')}"
            bench_files.append((cpp, label, key))

    for bench in sorted(BENCH_DIR.glob("bench_*.cpp")):
        name = bench.stem.removeprefix("bench_")
        name = "all-headers" if name == "all" else name
        # If the name matches an extra-include key, that key is required
        key = name if name in extra_includes else ""
        bench_files.append((bench, name, key))

    tests_dir = REPO_ROOT / "tests"
    for test in sorted(tests_dir.glob("test_*.cpp")):
        name = test.stem.removeprefix("test_")
        bench_files.append((test, name, ""))

    for spec in args.bench_file:
        parts = spec.split(":", 2)
        if len(parts) == 3:
            path, name, key = parts
        elif len(parts) == 2:
            path, name, key = parts[0], parts[1], ""
        else:
            continue
        bench_files.append((Path(path), name, key))

    results = []
    raw: dict[str, dict] = {}  # name -> samples, for --json
    for bench, name, include_key in bench_files:
        if include_key:
            inc_paths = extra_includes.get(include_key, [])
            missing = [p for p in inc_paths if not p.exists()]
            if not inc_paths or missing:
                print(f"  skipping  {name:<24}  (missing dep: {include_key})")
                continue
            xflags = [f"-I{p}" for p in inc_paths]
        else:
            xflags = []

        print(f"  measuring {name:<24} ...", end="", flush=True)
        sys_s = samples_ms(cxx, xflags, bench)
        psy_s = samples_ms(cxx, psychicstd_flags + xflags, bench)
        sys_ms = statistics.median(sys_s) if sys_s else None
        psy_ms = statistics.median(psy_s) if psy_s else None
        results.append((sys_ms, psy_ms, name))
        raw[name] = {
            "system_ms": sys_ms,
            "psychicstd_ms": psy_ms,
            "system_samples": sys_s,
            "psychicstd_samples": psy_s,
        }
        note = "" if psy_ms is not None else f"  {YELLOW}(psychicstd: n/a){RESET}"
        print(f" done{note}")

    # Sort by system time, but handle None (failed compile) gracefully
    results.sort(key=lambda r: r[0] if r[0] is not None else 0, reverse=True)

    print()
    print(f"{'name':<28}  {'system':>8}  {'psychicstd':>10}  {'speedup':>7}")
    print(f"{'----':<28}  {'------':>8}  {'----------':>10}  {'-------':>7}")
    for sys_ms, psy_ms, name in results:
        if psy_ms is not None:
            ratio = sys_ms / psy_ms
            psy_col = f"{psy_ms:>9.1f}ms"
            spd_col = f"{color(ratio)}{ratio:>6.1f}x{RESET}"
        else:
            psy_col = f"{YELLOW}{'n/a':>10}{RESET}"
            spd_col = f"{YELLOW}{'n/a':>7}{RESET}"
        sys_col = (
            f"{sys_ms:>7.1f}ms" if sys_ms is not None else f"{YELLOW}{'n/a':>8}{RESET}"
        )
        print(f"{name:<28}  {sys_col}  {psy_col}  {spd_col}")

    if args.json:
        # Machine-readable output for CI diffing (bench_diff.py). Keyed by the
        # benchmark name; medians are null when a config failed to compile, and
        # the raw per-rep samples let bench_diff put a CI on the change. The
        # reserved "__meta__" key records which compiler produced the numbers.
        raw["__meta__"] = {"compiler": cxx, "compiler_version": cxx_version}
        Path(args.json).write_text(json.dumps(raw, indent=2))
        print(f"\nWrote {args.json}")
        return

    speed_md = REPO_ROOT / "speed.md"
    with open(speed_md, "w") as f:
        f.write("# Compilation Speed\n\n")
        f.write(
            f"Median of {N} compilations per file. "
            "Ordered by system STL compile time, slowest first.\n\n"
        )
        f.write(
            f"🟢 above {THRESHOLD_GREEN}x  "
            f"🟡 {THRESHOLD_RED}x–{THRESHOLD_GREEN}x  "
            f"🔴 below {THRESHOLD_RED}x\n\n"
        )
        f.write(f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M')}\n\n")
        f.write("| | name | system | psychicstd | speedup |\n")
        f.write("|--|------|-------:|----------:|--------:|\n")
        for sys_ms, psy_ms, name in results:
            sys_cell = f"{sys_ms:.1f}ms" if sys_ms is not None else "n/a"
            if sys_ms is not None and psy_ms is not None:
                ratio = sys_ms / psy_ms
                psy_cell = f"{psy_ms:.1f}ms"
                spd_cell = f"{ratio:.1f}x"
                icon = emoji(ratio)
            else:
                psy_cell = "n/a"
                spd_cell = "n/a"
                icon = "⬜"
            f.write(f"| {icon} | {name} | {sys_cell} | {psy_cell} | {spd_cell} |\n")

    if subprocess.run(["which", "mdformat"], capture_output=True).returncode == 0:
        subprocess.run(["mdformat", speed_md], check=True)
    print(f"\nUpdated {speed_md}")


if __name__ == "__main__":
    main()
