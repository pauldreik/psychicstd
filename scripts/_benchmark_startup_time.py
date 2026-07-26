#!/usr/bin/env python3
"""
Measures exec-to-exit wall time for a representative small program built
against the system STL vs psychicstd, and compares the shared libraries loaded
by each executable. The timing includes dynamic loading, runtime initialization,
and the program's fixed workload; it is not an isolated dynamic-linker benchmark.

Writes results to stdout and updates startup.md in the repo root.
Usage: _benchmark_startup_time.py [system_binary] [psychicstd_binary]
"""

import os
import random
import statistics
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
BENCH_DIR = REPO_ROOT / "benchmarks" / "startup_time"
N = int(os.environ.get("BENCH_N", "300"))
REPETITIONS = int(os.environ.get("BENCH_REPS", "3"))
WARMUP = 20
INACCURATE = os.environ.get("BENCH_INACCURATE") == "1"


def run_once_ms(binary: Path) -> float:
    start = time.perf_counter()
    subprocess.run(
        [str(binary)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    return (time.perf_counter() - start) * 1000


def paired_samples_ms(
    system: Path, psychicstd: Path
) -> tuple[list[float], list[float]]:
    system_samples = []
    psychicstd_samples = []
    for i in range(WARMUP + N):
        pair = (system, psychicstd) if i % 2 == 0 else (psychicstd, system)
        measured = {binary: run_once_ms(binary) for binary in pair}
        if i >= WARMUP:
            system_samples.append(round(measured[system], 4))
            psychicstd_samples.append(round(measured[psychicstd], 4))
    return system_samples, psychicstd_samples


def bootstrap_speedup_ci(
    batches: list[tuple[list[float], list[float]]],
    iterations: int = 2000,
    seed: int = 12345,
) -> tuple[float, float] | None:
    if not batches or any(len(system) < 2 for system, _ in batches):
        return None
    rnd = random.Random(seed)
    ratios = []
    for _ in range(iterations):
        medians = []
        for system, psychicstd in batches:
            indices = [rnd.randrange(len(system)) for _ in system]
            medians.append(
                (
                    statistics.median(system[index] for index in indices),
                    statistics.median(psychicstd[index] for index in indices),
                )
            )
        system_median = statistics.median(sample[0] for sample in medians)
        psychicstd_median = statistics.median(sample[1] for sample in medians)
        ratios.append(system_median / psychicstd_median)
    ratios.sort()
    return (
        ratios[int(0.025 * (len(ratios) - 1))],
        ratios[int(0.975 * (len(ratios) - 1))],
    )


def shared_libs(binary: Path) -> list[str]:
    if sys.platform == "darwin":
        out = subprocess.run(
            ["otool", "-L", str(binary)],
            capture_output=True,
            text=True,
            check=False,
        ).stdout
        # First line is the binary itself; the rest are "\t/path/lib.dylib (...)".
        libs = []
        for line in out.splitlines()[1:]:
            line = line.strip()
            if not line:
                continue
            libs.append(line.split()[0].rsplit("/", 1)[-1])
        return sorted(libs)
    out = subprocess.run(
        ["ldd", str(binary)], capture_output=True, text=True, check=False
    ).stdout
    libs = []
    for line in out.splitlines():
        line = line.strip()
        name = line.split()[0]
        if name in ("linux-vdso.so.1",) or name.startswith("/lib64/ld-linux"):
            continue
        libs.append(name)
    return sorted(libs)


def main() -> None:
    system_bin = Path(
        sys.argv[1]
        if len(sys.argv) > 1
        else REPO_ROOT / "build/benchmarks/startup_time/bench_startup_system"
    )
    psychicstd_bin = Path(
        sys.argv[2]
        if len(sys.argv) > 2
        else REPO_ROOT / "build/benchmarks/startup_time/bench_startup_psychicstd"
    )
    for b in (system_bin, psychicstd_bin):
        if not b.is_file():
            sys.exit(f"error: {b} not found -- build it first (cmake --build build)")

    print(
        f"measuring {REPETITIONS} batches of {N} paired runs "
        f"(after {WARMUP} warmup pairs per batch)..."
    )
    batch_medians = []
    batches = []
    for repetition in range(REPETITIONS):
        sys_s, psy_s = paired_samples_ms(system_bin, psychicstd_bin)
        batches.append((sys_s, psy_s))
        batch_medians.append((statistics.median(sys_s), statistics.median(psy_s)))
        sys_batch, psy_batch = batch_medians[-1]
        print(
            f"batch {repetition + 1}: system {sys_batch:.3f} ms, "
            f"psychicstd {psy_batch:.3f} ms, {sys_batch / psy_batch:.2f}x"
        )
    sys_ms = statistics.median(sample[0] for sample in batch_medians)
    psy_ms = statistics.median(sample[1] for sample in batch_medians)
    ratio = sys_ms / psy_ms
    speedup_ci = bootstrap_speedup_ci(batches)

    sys_libs = shared_libs(system_bin)
    psy_libs = shared_libs(psychicstd_bin)

    print(f"system:     median {sys_ms:.3f} ms, libs: {', '.join(sys_libs)}")
    print(f"psychicstd: median {psy_ms:.3f} ms, libs: {', '.join(psy_libs)}")
    ci_text = f"[{speedup_ci[0]:.2f}x, {speedup_ci[1]:.2f}x]" if speedup_ci else "n/a"
    print(f"speedup: {ratio:.2f}x, 95% CI: {ci_text}")

    startup_md = REPO_ROOT / "startup.md"
    with open(startup_md, "w") as f:
        f.write("# Process Startup Speed\n\n")
        f.write(
            f"Median of {REPETITIONS} batches of {N} paired runs (after {WARMUP} "
            "warmup pairs per batch) of a representative small program "
            "(`benchmarks/startup_time/bench_startup.cpp`) linked against the system "
            "STL vs psychicstd. This measures exec-to-exit wall time, including "
            "dynamic loading, runtime initialization, and the program's fixed "
            "workload. It is not an isolated measurement of dynamic-linker time.\n\n"
        )
        f.write(
            "psychicstd is linked as a static archive: required archive members are "
            "copied into the executable, so `libpsychicstd.a` is not a startup-time "
            "shared-library dependency. The table lists shared libraries reported by "
            "the platform dependency tool.\n\n"
        )
        f.write(
            "The interval is a paired bootstrap 95% confidence interval. Each "
            "bootstrap sample resamples the paired runs within each batch.\n\n"
        )
        f.write(
            f"Last updated: {datetime.now().astimezone().strftime('%Y-%m-%d %H:%M')}\n\n"
        )
        if INACCURATE:
            f.write(
                "**This smoke-test run used minimal sampling and ccache, so its "
                "timings are not representative.**\n\n"
            )
        f.write("| | median exec-to-exit | shared libraries |\n")
        f.write("|--|---:|---|\n")
        f.write(f"| system | {sys_ms:.3f} ms | {', '.join(sys_libs)} |\n")
        f.write(f"| psychicstd | {psy_ms:.3f} ms | {', '.join(psy_libs)} |\n")
        f.write(f"\nSpeedup: **{ratio:.2f}x** (95% CI: **{ci_text}**)\n")
        f.write(
            "\n---\n\nReproduce on your machine:\n\n"
            "`cmake -B build/ -S . -DCMAKE_BUILD_TYPE=Debug`\n\n"
            "`cmake --build build/ --target startup_bench`\n"
        )

    if (
        subprocess.run(
            ["which", "mdformat"], capture_output=True, check=False
        ).returncode
        == 0
    ):
        subprocess.run(["mdformat", startup_md], check=True)
    print(f"\nUpdated {startup_md}")


if __name__ == "__main__":
    main()
