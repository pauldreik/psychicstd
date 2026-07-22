#!/usr/bin/env python3
"""
Measures exec-to-exit wall time for a representative small program built
against the system STL vs psychicstd, and compares the shared libraries loaded
by each executable. The timing includes dynamic loading, runtime initialization,
and the program's fixed workload; it is not an isolated dynamic-linker benchmark.

Writes results to stdout and updates startup.md in the repo root.
Usage: run_bench.py [system_binary] [psychicstd_binary]
"""

import os
import statistics
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path

BENCH_DIR = Path(__file__).parent.resolve()
REPO_ROOT = BENCH_DIR.parent.parent
N = int(os.environ.get("BENCH_N", "300"))
REPETITIONS = int(os.environ.get("BENCH_REPS", "3"))
WARMUP = 20


def run_once_ms(binary: Path) -> float:
    start = time.perf_counter()
    subprocess.run([str(binary)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
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


def shared_libs(binary: Path) -> list[str]:
    if sys.platform == "darwin":
        out = subprocess.run(
            ["otool", "-L", str(binary)], capture_output=True, text=True
        ).stdout
        # First line is the binary itself; the rest are "\t/path/lib.dylib (...)".
        libs = []
        for line in out.splitlines()[1:]:
            line = line.strip()
            if not line:
                continue
            libs.append(line.split()[0].rsplit("/", 1)[-1])
        return sorted(libs)
    out = subprocess.run(["ldd", str(binary)], capture_output=True, text=True).stdout
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
    for repetition in range(REPETITIONS):
        sys_s, psy_s = paired_samples_ms(system_bin, psychicstd_bin)
        batch_medians.append((statistics.median(sys_s), statistics.median(psy_s)))
        sys_batch, psy_batch = batch_medians[-1]
        print(
            f"batch {repetition + 1}: system {sys_batch:.3f} ms, "
            f"psychicstd {psy_batch:.3f} ms, {sys_batch / psy_batch:.2f}x"
        )
    sys_ms = statistics.median(sample[0] for sample in batch_medians)
    psy_ms = statistics.median(sample[1] for sample in batch_medians)
    ratio = sys_ms / psy_ms

    sys_libs = shared_libs(system_bin)
    psy_libs = shared_libs(psychicstd_bin)

    print(f"system:     median {sys_ms:.3f} ms, libs: {', '.join(sys_libs)}")
    print(f"psychicstd: median {psy_ms:.3f} ms, libs: {', '.join(psy_libs)}")
    print(f"speedup: {ratio:.2f}x")

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
        f.write(f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M')}\n\n")
        f.write("| | median exec-to-exit | shared libraries |\n")
        f.write("|--|---:|---|\n")
        f.write(f"| system | {sys_ms:.3f} ms | {', '.join(sys_libs)} |\n")
        f.write(f"| psychicstd | {psy_ms:.3f} ms | {', '.join(psy_libs)} |\n")
        f.write(f"\nSpeedup: **{ratio:.2f}x**\n")

    if subprocess.run(["which", "mdformat"], capture_output=True).returncode == 0:
        subprocess.run(["mdformat", startup_md], check=True)
    print(f"\nUpdated {startup_md}")


if __name__ == "__main__":
    main()
