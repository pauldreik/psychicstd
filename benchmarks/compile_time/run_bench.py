#!/usr/bin/env python3
"""
Measures median compile time for each benchmark file, system STL vs psychicstd.
Writes results to stdout and updates speed.md in the repo root.
Usage: run_bench.py [cxx_compiler] [psychicstd_include_dir]
       [-n REPS]
       [--extra-include name:path ...]
       [--bench-file path:name:include_key ...]
"""

import argparse
import json
import os
import random
import statistics
import subprocess
import tempfile
import time
from datetime import datetime
from pathlib import Path

os.environ["CCACHE_DISABLE"] = "1"

BENCH_DIR = Path(__file__).parent.resolve()
REPO_ROOT = BENCH_DIR.parent.parent
DEFAULT_REPS = int(os.environ.get("BENCH_N", "10"))

THRESHOLD_GREEN = 1.2  # above this: psychicstd is meaningfully faster
THRESHOLD_RED = 0.8  # below this: psychicstd is meaningfully slower

RED = "\033[31m"
YELLOW = "\033[33m"
GREEN = "\033[32m"
DIM = "\033[2m"
RESET = "\033[0m"


def generate_all_headers_benchmark(include_dir: Path, output_dir: Path) -> Path:
    headers = sorted(
        path.name
        for path in include_dir.iterdir()
        if path.is_file()
        and not path.name.startswith("__psychicstd")
        and path.name != "cxxabi.h"
    )
    if not headers:
        raise RuntimeError(f"no public headers found in {include_dir}")
    source = output_dir / "all_headers.cpp"
    source.write_text("".join(f"#include <{header}>\n" for header in headers))
    return source


def compile_ms(cxx: str, flags: list[str], file: Path) -> float:
    cmd = [cxx, *flags, "-std=c++23", "-c", str(file), "-o", "/dev/null"]
    start = time.perf_counter()
    subprocess.run(cmd, check=True, stderr=subprocess.DEVNULL)
    return (time.perf_counter() - start) * 1000


def samples_ms(
    cxx: str, flags: list[str], file: Path, repetitions: int
) -> list[float] | None:
    """Returns the list of per-rep compile times in ms, or None if the file
    fails to compile with these flags. Keeping the raw samples (not just the
    median) lets the diff tool put a confidence interval on the change."""
    try:
        return [round(compile_ms(cxx, flags, file), 3) for _ in range(repetitions)]
    except subprocess.CalledProcessError:
        return None


def median_ms(cxx: str, flags: list[str], file: Path, repetitions: int) -> float | None:
    s = samples_ms(cxx, flags, file, repetitions)
    return statistics.median(s) if s else None


def bootstrap_speedup_ci(
    system_samples: list[float],
    psychicstd_samples: list[float],
    iterations: int = 2000,
    seed: int = 12345,
) -> tuple[float, float] | None:
    """Return a bootstrap 95% CI for system/psychicstd median speedup."""
    if len(system_samples) < 2 or len(psychicstd_samples) < 2:
        return None
    rnd = random.Random(seed)
    system_count = len(system_samples)
    psychicstd_count = len(psychicstd_samples)
    ratios = []
    for _ in range(iterations):
        system_median = statistics.median(
            rnd.choice(system_samples) for _ in range(system_count)
        )
        psychicstd_median = statistics.median(
            rnd.choice(psychicstd_samples) for _ in range(psychicstd_count)
        )
        if psychicstd_median:
            ratios.append(system_median / psychicstd_median)
    if not ratios:
        return None
    ratios.sort()
    return (
        ratios[int(0.025 * (len(ratios) - 1))],
        ratios[int(0.975 * (len(ratios) - 1))],
    )


def format_ci(ci: tuple[float, float] | None) -> str:
    return f"[{ci[0]:.2f}x, {ci[1]:.2f}x]" if ci else "n/a"


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
        "-n",
        "--reps",
        type=int,
        default=DEFAULT_REPS,
        help=f"compilations per file (default: {DEFAULT_REPS})",
    )
    parser.add_argument(
        "--extra-include",
        action="append",
        default=[],
        metavar="name:path",
        help="Extra include path for a named key (e.g. thirdparty:/path/to/include)",
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
    if args.reps < 1:
        parser.error("--reps must be at least 1")

    psychicstd_include = Path(args.psychicstd_inc).resolve()
    if not psychicstd_include.is_dir():
        parser.error(f"psychicstd include directory not found: {psychicstd_include}")
    aggregate_dir = tempfile.TemporaryDirectory(prefix="psychicstd-all-headers-")
    aggregate_bench = generate_all_headers_benchmark(
        psychicstd_include, Path(aggregate_dir.name)
    )

    # extra_includes maps name -> list of paths (colon-separated paths after name:)
    extra_includes: dict[str, list[Path]] = {}
    for spec in args.extra_include:
        name, _, paths_str = spec.partition(":")
        if name and paths_str:
            extra_includes[name] = [Path(p) for p in paths_str.split(":") if p]

    cxx = args.compiler
    psychicstd_flags = ["-nostdinc++", f"-I{psychicstd_include}"]

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
    tests_dir = REPO_ROOT / "tests"

    bench_files.append((aggregate_bench, "all-headers", ""))

    # Auto-discover real-code fixture dirs (benchmarks/compile_time/<name>/*.cpp)
    # whose name matches a known extra_include key (e.g. thirdparty). These are
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
        if (tests_dir / f"test_{name}.cpp").is_file():
            name = f"bench/{name}"
        # If the name matches an extra-include key, that key is required
        key = name if name in extra_includes else ""
        bench_files.append((bench, name, key))

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
        sys_s = samples_ms(cxx, xflags, bench, args.reps)
        psy_s = samples_ms(cxx, psychicstd_flags + xflags, bench, args.reps)
        sys_ms = statistics.median(sys_s) if sys_s else None
        psy_ms = statistics.median(psy_s) if psy_s else None
        speedup_ci = bootstrap_speedup_ci(sys_s, psy_s) if sys_s and psy_s else None
        results.append((sys_ms, psy_ms, speedup_ci, name))
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
    print(
        f"{'name':<28}  {'system':>8}  {'psychicstd':>10}  "
        f"{'speedup':>7}  {'95% CI':>16}"
    )
    print(
        f"{'----':<28}  {'------':>8}  {'----------':>10}  "
        f"{'-------':>7}  {'------':>16}"
    )
    for sys_ms, psy_ms, speedup_ci, name in results:
        if psy_ms is not None:
            ratio = sys_ms / psy_ms
            psy_col = f"{psy_ms:>9.1f}ms"
            spd_col = f"{color(ratio)}{ratio:>6.2f}x{RESET}"
        else:
            psy_col = f"{YELLOW}{'n/a':>10}{RESET}"
            spd_col = f"{YELLOW}{'n/a':>7}{RESET}"
        sys_col = (
            f"{sys_ms:>7.1f}ms" if sys_ms is not None else f"{YELLOW}{'n/a':>8}{RESET}"
        )
        print(
            f"{name:<28}  {sys_col}  {psy_col}  {spd_col}  {format_ci(speedup_ci):>16}"
        )

    if args.json:
        # Machine-readable output for CI diffing (bench_diff.py). Keyed by the
        # benchmark name; medians are null when a config failed to compile, and
        # the raw per-rep samples let bench_diff put a CI on the change. The
        # reserved "__meta__" key records which compiler produced the numbers.
        raw["__meta__"] = {
            "compiler": cxx,
            "compiler_version": cxx_version,
            "repetitions": args.reps,
        }
        Path(args.json).write_text(json.dumps(raw, indent=2))
        print(f"\nWrote {args.json}")
        aggregate_dir.cleanup()
        return

    speed_md = REPO_ROOT / "speed.md"
    with open(speed_md, "w") as f:
        f.write("# Compilation Speed\n\n")
        f.write(
            f"Median of {args.reps} compilations per file. "
            "Ordered by system STL compile time, slowest first.\n\n"
        )
        f.write(f"Compiler: `{cxx_version}`\n\n")
        f.write(
            "The interval is a bootstrapped 95% confidence interval for the "
            "system/psychicstd speedup ratio.\n\n"
        )
        f.write(
            f"🟢 above {THRESHOLD_GREEN}x  "
            f"🟡 {THRESHOLD_RED}x–{THRESHOLD_GREEN}x  "
            f"🔴 below {THRESHOLD_RED}x\n\n"
        )
        f.write(f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M')}\n\n")
        f.write("| | name | system | psychicstd | speedup | 95% CI |\n")
        f.write("|--|------|-------:|----------:|--------:|-------:|\n")
        for sys_ms, psy_ms, speedup_ci, name in results:
            sys_cell = f"{sys_ms:.1f}ms" if sys_ms is not None else "n/a"
            if sys_ms is not None and psy_ms is not None:
                ratio = sys_ms / psy_ms
                psy_cell = f"{psy_ms:.1f}ms"
                spd_cell = f"{ratio:.2f}x"
                icon = emoji(ratio)
            else:
                psy_cell = "n/a"
                spd_cell = "n/a"
                icon = "⬜"
            f.write(
                f"| {icon} | {name} | {sys_cell} | {psy_cell} | {spd_cell} | "
                f"{format_ci(speedup_ci)} |\n"
            )

    if subprocess.run(["which", "mdformat"], capture_output=True).returncode == 0:
        subprocess.run(["mdformat", speed_md], check=True)
    aggregate_dir.cleanup()
    print(f"\nUpdated {speed_md}")


if __name__ == "__main__":
    main()
