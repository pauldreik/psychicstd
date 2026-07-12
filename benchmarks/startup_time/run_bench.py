#!/usr/bin/env python3
"""
Measures process startup time (median exec-to-exit wall time) for a trivial
program built against the system STL vs psychicstd, and compares the shared
libraries each links against. psychicstd builds link fewer shared objects
(no libstdc++.so.6/libm.so.6), so the dynamic loader has less work to do on
every process start -- distinct from and additional to the compile-time win.

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
WARMUP = 20


def run_once_ms(binary: Path) -> float:
    start = time.perf_counter()
    subprocess.run([str(binary)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return (time.perf_counter() - start) * 1000


def samples_ms(binary: Path) -> list[float]:
    for _ in range(WARMUP):
        run_once_ms(binary)
    return [round(run_once_ms(binary), 4) for _ in range(N)]


def shared_libs(binary: Path) -> list[str]:
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

    print(f"measuring {N} runs each (after {WARMUP} warmup runs)...")
    sys_s = samples_ms(system_bin)
    psy_s = samples_ms(psychicstd_bin)
    sys_ms = statistics.median(sys_s)
    psy_ms = statistics.median(psy_s)
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
            f"Median of {N} runs (after {WARMUP} warmup runs) of a trivial program "
            "(`benchmarks/startup_time/bench_startup.cpp`) linked against the system "
            "STL vs psychicstd. This measures exec-to-exit wall time, i.e. dynamic "
            "linker + startup overhead -- separate from compile time, and separate "
            "from the program's own work.\n\n"
        )
        f.write(f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M')}\n\n")
        f.write("| | median startup | shared libraries |\n")
        f.write("|--|---:|---|\n")
        f.write(f"| system | {sys_ms:.3f} ms | {', '.join(sys_libs)} |\n")
        f.write(f"| psychicstd | {psy_ms:.3f} ms | {', '.join(psy_libs)} |\n")
        f.write(f"\nSpeedup: **{ratio:.2f}x**\n")

    if subprocess.run(["which", "mdformat"], capture_output=True).returncode == 0:
        subprocess.run(["mdformat", startup_md], check=True)
    print(f"\nUpdated {startup_md}")


if __name__ == "__main__":
    main()
