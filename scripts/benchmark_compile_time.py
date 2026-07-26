#!/usr/bin/env python3
"""Measure focused compile-time workloads, system STL vs psychicstd.

Writes results to stdout and updates compile_time.md in the repo root.
Usage: benchmark_compile_time.py [cxx_compiler] [-n REPS]
       [--include psychicstd_include_dir]
       [--extra-include name:path ...]
       [--bench-file path:name:include_key ...]
"""

import argparse
import json
import os
import random
import shlex
import statistics
import subprocess
import tempfile
import time
from datetime import datetime
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
BENCH_DIR = REPO_ROOT / "benchmarks" / "compile_time"
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


def compile_sample(cxx: str, flags: list[str], file: Path) -> tuple[float, int]:
    cmd = [cxx, *flags, "-std=c++20", "-c", str(file), "-o", "/dev/null"]
    with tempfile.NamedTemporaryFile() as rss_file:
        measured = ["/usr/bin/time", "-f", "%M", "-o", rss_file.name, *cmd]
        start = time.perf_counter()
        subprocess.run(measured, check=True, stderr=subprocess.DEVNULL)
        elapsed_ms = (time.perf_counter() - start) * 1000
        rss_file.seek(0)
        return elapsed_ms, int(rss_file.read())


def samples(
    cxx: str, flags: list[str], file: Path, repetitions: int
) -> tuple[list[float], list[int]] | None:
    """Return per-compilation elapsed milliseconds and peak RSS in KiB."""
    try:
        measured = [compile_sample(cxx, flags, file) for _ in range(repetitions)]
        return (
            [round(elapsed_ms, 3) for elapsed_ms, _ in measured],
            [rss_kib for _, rss_kib in measured],
        )
    except subprocess.CalledProcessError:
        return None


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


def bootstrap_reduction_ci(
    baseline_samples: list[float],
    improved_samples: list[float],
    iterations: int = 2000,
    seed: int = 12345,
) -> tuple[float, float] | None:
    if len(baseline_samples) < 2 or len(improved_samples) < 2:
        return None
    rnd = random.Random(seed)
    baseline_count = len(baseline_samples)
    improved_count = len(improved_samples)
    reductions = []
    for _ in range(iterations):
        baseline = statistics.median(
            rnd.choice(baseline_samples) for _ in range(baseline_count)
        )
        improved = statistics.median(
            rnd.choice(improved_samples) for _ in range(improved_count)
        )
        if baseline:
            reductions.append((baseline - improved) / baseline * 100)
    if not reductions:
        return None
    reductions.sort()
    return (
        reductions[int(0.025 * (len(reductions) - 1))],
        reductions[int(0.975 * (len(reductions) - 1))],
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
    parser.add_argument(
        "--include",
        type=Path,
        default=REPO_ROOT / "include",
        help="psychicstd include directory (default: repository include/)",
    )
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
        help="Write per-file results as JSON to PATH (for CI diffing) and skip compile_time.md",
    )
    parser.add_argument(
        "--enable-ccache",
        action="store_true",
        help="leave ccache enabled for a fast, non-representative smoke test",
    )
    args = parser.parse_args()
    if args.reps < 1:
        parser.error("--reps must be at least 1")
    if not args.enable_ccache:
        os.environ["CCACHE_DISABLE"] = "1"

    psychicstd_include = args.include.resolve()
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
    psychicstd_dropin_flags = ["-nostdinc++", f"-I{psychicstd_include}"]
    psychicstd_strict_flags = [
        *psychicstd_dropin_flags,
        "-D_PSYCHICSTD_COMPATIBILITY_LEVEL=0",
    ]

    try:
        cxx_version = subprocess.run(
            [cxx, "--version"], capture_output=True, text=True, check=False
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
    bench_files.append((aggregate_bench, "all-headers", ""))
    bench_files.append((REPO_ROOT / "examples" / "wordcounter.cpp", "wordcounter", ""))

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
        # If the name matches an extra-include key, that key is required
        key = name if name in extra_includes else ""
        bench_files.append((bench, name, key))

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
        system = samples(cxx, xflags, bench, args.reps)
        strict = samples(cxx, psychicstd_strict_flags + xflags, bench, args.reps)
        dropin = samples(cxx, psychicstd_dropin_flags + xflags, bench, args.reps)
        sys_s, sys_rss = system if system else (None, None)
        strict_s, strict_rss = strict if strict else (None, None)
        dropin_s, dropin_rss = dropin if dropin else (None, None)
        sys_ms = statistics.median(sys_s) if sys_s else None
        strict_ms = statistics.median(strict_s) if strict_s else None
        dropin_ms = statistics.median(dropin_s) if dropin_s else None
        sys_rss_kib = statistics.median(sys_rss) if sys_rss else None
        strict_rss_kib = statistics.median(strict_rss) if strict_rss else None
        dropin_rss_kib = statistics.median(dropin_rss) if dropin_rss else None
        strict_ci = (
            bootstrap_speedup_ci(sys_s, strict_s) if sys_s and strict_s else None
        )
        dropin_ci = (
            bootstrap_speedup_ci(sys_s, dropin_s) if sys_s and dropin_s else None
        )
        results.append(
            (
                sys_ms,
                strict_ms,
                dropin_ms,
                sys_rss_kib,
                strict_rss_kib,
                dropin_rss_kib,
                strict_ci,
                dropin_ci,
                name,
            )
        )
        raw[name] = {
            "system_ms": sys_ms,
            "psychicstd_ms": dropin_ms,
            "psychicstd_strict_ms": strict_ms,
            "system_samples": sys_s,
            "psychicstd_samples": dropin_s,
            "psychicstd_strict_samples": strict_s,
            "system_peak_rss_kib": sys_rss_kib,
            "psychicstd_peak_rss_kib": dropin_rss_kib,
            "psychicstd_strict_peak_rss_kib": strict_rss_kib,
        }
        failed_modes = [
            mode
            for mode, value in (("strict", strict_ms), ("drop-in", dropin_ms))
            if value is None
        ]
        note = (
            f"  {YELLOW}({', '.join(failed_modes)}: n/a){RESET}" if failed_modes else ""
        )
        print(f" done{note}")

    # Sort by system time, but handle None (failed compile) gracefully
    results.sort(key=lambda r: r[0] if r[0] is not None else 0, reverse=True)

    print()
    print(
        f"{'name':<28}  {'mode':<7}  {'system':>8}  {'psychicstd':>10}  "
        f"{'speedup':>7}  {'95% CI':>16}"
    )
    print(
        f"{'----':<28}  {'----':<7}  {'------':>8}  {'----------':>10}  "
        f"{'-------':>7}  {'------':>16}"
    )
    for sys_ms, strict_ms, dropin_ms, _, _, _, strict_ci, dropin_ci, name in results:
        for mode, psy_ms, speedup_ci in (
            ("strict", strict_ms, strict_ci),
            ("drop-in", dropin_ms, dropin_ci),
        ):
            if sys_ms is not None and psy_ms is not None:
                ratio = sys_ms / psy_ms
                psy_col = f"{psy_ms:>9.1f}ms"
                spd_col = f"{color(ratio)}{ratio:>6.2f}x{RESET}"
            else:
                psy_col = f"{YELLOW}{'n/a':>10}{RESET}"
                spd_col = f"{YELLOW}{'n/a':>7}{RESET}"
            sys_col = (
                f"{sys_ms:>7.1f}ms"
                if sys_ms is not None
                else f"{YELLOW}{'n/a':>8}{RESET}"
            )
            print(
                f"{name:<28}  {mode:<7}  {sys_col}  {psy_col}  "
                f"{spd_col}  {format_ci(speedup_ci):>16}"
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
            "psychicstd_modes": ["strict", "drop-in"],
        }
        Path(args.json).write_text(json.dumps(raw, indent=2))
        print(f"\nWrote {args.json}")
        aggregate_dir.cleanup()
        return

    report = REPO_ROOT / "compile_time.md"
    with open(report, "w") as f:
        f.write("# Focused Compile-Time Benchmarks\n\n")
        f.write(
            f"Median elapsed time and peak compiler RSS from {args.reps} "
            "compilations per workload. "
            "Ordered by system STL compile time, slowest first.\n\n"
        )
        f.write(
            "Each psychicstd workload is measured in strict and drop-in mode; "
            "the system workload is compiled once and shared by both rows.\n\n"
        )
        f.write(f"Compiler: `{cxx_version}`\n\n")
        f.write("Language mode: `C++20`\n\n")
        f.write(
            "The interval is a bootstrapped 95% confidence interval for the "
            "system/psychicstd speedup ratio.\n\n"
        )
        f.write(
            f"🟢 above {THRESHOLD_GREEN}x  "
            f"🟡 {THRESHOLD_RED}x–{THRESHOLD_GREEN}x  "
            f"🔴 below {THRESHOLD_RED}x\n\n"
        )
        f.write(
            f"Last updated: {datetime.now().astimezone().strftime('%Y-%m-%d %H:%M')}\n\n"
        )
        if args.enable_ccache:
            f.write(
                "**ccache was enabled, so these smoke-test timings are not "
                "representative.**\n\n"
            )
        f.write(
            "| | name | mode | system | psychicstd | speedup | 95% CI | "
            "system peak RSS | psychicstd peak RSS |\n"
        )
        f.write(
            "|--|------|------|-------:|----------:|--------:|-------:|"
            "---------------:|--------------------:|\n"
        )
        for (
            sys_ms,
            strict_ms,
            dropin_ms,
            sys_rss_kib,
            strict_rss_kib,
            dropin_rss_kib,
            strict_ci,
            dropin_ci,
            name,
        ) in results:
            for mode, psy_ms, psy_rss_kib, speedup_ci in (
                ("strict", strict_ms, strict_rss_kib, strict_ci),
                ("drop-in", dropin_ms, dropin_rss_kib, dropin_ci),
            ):
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
                sys_rss_cell = (
                    f"{sys_rss_kib / 1024:.1f} MiB"
                    if sys_rss_kib is not None
                    else "n/a"
                )
                psy_rss_cell = (
                    f"{psy_rss_kib / 1024:.1f} MiB"
                    if psy_rss_kib is not None
                    else "n/a"
                )
                f.write(
                    f"| {icon} | {name} | {mode} | {sys_cell} | {psy_cell} | "
                    f"{spd_cell} | {format_ci(speedup_ci)} | {sys_rss_cell} | "
                    f"{psy_rss_cell} |\n"
                )
        f.write(
            "\n## Strict-mode improvement\n\n"
            "Reduction relative to psychicstd's drop-in mode; positive values "
            "mean strict mode uses less time or memory.\n\n"
            "| workload | compile-time reduction | 95% CI | peak-RSS reduction |\n"
            "|---|---:|---:|---:|\n"
        )
        for (
            _,
            strict_ms,
            dropin_ms,
            _,
            strict_rss_kib,
            dropin_rss_kib,
            _,
            _,
            name,
        ) in results:
            if strict_ms is not None and dropin_ms is not None and dropin_ms:
                reduction = (dropin_ms - strict_ms) / dropin_ms * 100
                reduction_cell = f"{reduction:.1f}%"
                reduction_ci = bootstrap_reduction_ci(
                    raw[name]["psychicstd_samples"],
                    raw[name]["psychicstd_strict_samples"],
                )
                reduction_ci_cell = (
                    f"[{reduction_ci[0]:.1f}%, {reduction_ci[1]:.1f}%]"
                    if reduction_ci
                    else "n/a"
                )
            else:
                reduction_cell = "n/a"
                reduction_ci_cell = "n/a"
            rss_reduction_cell = (
                f"{(dropin_rss_kib - strict_rss_kib) / dropin_rss_kib * 100:.1f}%"
                if strict_rss_kib is not None
                and dropin_rss_kib is not None
                and dropin_rss_kib
                else "n/a"
            )
            f.write(
                f"| {name} | {reduction_cell} | {reduction_ci_cell} | "
                f"{rss_reduction_cell} |\n"
            )
        reproduce = [
            "scripts/benchmark_compile_time.py",
            shlex.quote(args.compiler),
            "--reps",
            str(args.reps),
        ]
        if psychicstd_include != (REPO_ROOT / "include").resolve():
            reproduce.extend(("--include", shlex.quote(str(psychicstd_include))))
        for spec in args.extra_include:
            reproduce.extend(("--extra-include", shlex.quote(spec)))
        for spec in args.bench_file:
            reproduce.extend(("--bench-file", shlex.quote(spec)))
        if args.enable_ccache:
            reproduce.append("--enable-ccache")
        f.write("\n---\n\nReproduce on your machine:\n\n```bash\n")
        f.write(f"{' '.join(reproduce)}\n```\n")

    if (
        subprocess.run(
            ["which", "mdformat"], capture_output=True, check=False
        ).returncode
        == 0
    ):
        subprocess.run(["mdformat", report], check=True)
    aggregate_dir.cleanup()
    print(f"\nUpdated {report}")


if __name__ == "__main__":
    main()
