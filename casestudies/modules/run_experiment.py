#!/usr/bin/env python3
"""Measure how a C++20 header unit (module) affects compile time of a <string>
using translation unit, for both psychicstd and the system libstdc++.

A header unit is the module-world analog of a precompiled header: an existing
header is compiled once into a binary module interface (BMI, a .pcm file) and
then `import`ed instead of `#include`d. Neither psychicstd nor libstdc++ is
written as named modules, so header units are the fair comparison.

For each standard library we measure:
  baseline   - compile tu_include.cpp normally (#include <string>)
  hu-build   - one-time cost of building the header unit from pch.h
  with-hu    - compile tu_mod.cpp (import "pch.h";) using the built .pcm

Then we ask the same questions as the PCH study, so the two can be compared:
  - how much does a header unit speed up a single TU, per config?
  - does psychicstd still beat libstdc++ once both use modules?
  - how many TUs before the one-time build pays off?

Scoped to <string> only (the most complete part of psychicstd) for fairness.

Usage:  python3 run_experiment.py
Env:    CXX (default clang++-21), N (reps)
"""

import os
import statistics
import subprocess
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
INCLUDE = os.path.join(os.path.dirname(os.path.dirname(HERE)), "include")
CXX = os.environ.get("CXX", "clang++-21")
STD = os.environ.get("STD", "c++20")
N = int(os.environ.get("N", "12"))
WARN = "-Wno-experimental-header-units"

CONFIGS = {
    "system": [],
    "psychicstd": ["-nostdinc++", "-isystem", INCLUDE],
}


def run_ms(cmd):
    start = time.perf_counter()
    subprocess.run(cmd, check=True)
    return (time.perf_counter() - start) * 1000.0


def median_ms(cmd):
    run_ms(cmd)  # warm
    return statistics.median(sorted(run_ms(cmd) for _ in range(N)))


def measure(label, flags, workdir):
    base = [CXX, *flags, f"-std={STD}", WARN]
    tu_inc = os.path.join(HERE, "tu_include.cpp")
    tu_mod = os.path.join(HERE, "tu_mod.cpp")
    hdr = os.path.join(HERE, "pch.h")
    pcm = os.path.join(workdir, f"{label}.pcm")
    build_cmd = [*base, "-fmodule-header=user", "-xc++-user-header", hdr, "-o", pcm]

    baseline = median_ms([*base, "-c", tu_inc, "-o", "/dev/null"])
    hu_build = median_ms(build_cmd)
    subprocess.run(build_cmd, check=True)  # ensure it exists for the with-hu runs
    size = os.path.getsize(pcm)
    with_hu = median_ms(
        [*base, f"-fmodule-file={pcm}", "-c", tu_mod, "-o", "/dev/null"]
    )
    return baseline, hu_build, with_hu, size


def main():
    ver = subprocess.run(
        [CXX, "--version"], capture_output=True, text=True
    ).stdout.splitlines()[0]
    print(f"compiler: {ver}")
    print(f"standard: {STD}, reps: {N}\n")

    workdir = tempfile.mkdtemp(prefix="modules_")
    rows = {label: measure(label, flags, workdir) for label, flags in CONFIGS.items()}

    print("=== per-config results (ms) ===")
    print(
        f"  {'config':<12}{'baseline':>10}{'hu-build':>10}{'with-hu':>10}{'bmi size':>12}"
    )
    for label, (base, build, withhu, size) in rows.items():
        print(
            f"  {label:<12}{base:>10.1f}{build:>10.1f}{withhu:>10.1f}{size / 1024:>10.0f}KB"
        )

    print("\n=== does a header unit help each config? ===")
    for label, (base, build, withhu, size) in rows.items():
        print(
            f"  {label:<12}: {base:.1f} -> {withhu:.1f} ms per TU  ({base / withhu:.2f}x faster)"
        )

    print("\n=== does psychicstd still win with modules? ===")
    sb, _, sw, _ = rows["system"]
    pb, _, pw, _ = rows["psychicstd"]
    print(
        f"  baseline    : psychicstd {pb:.1f} ms vs system {sb:.1f} ms  ({sb / pb:.2f}x)"
    )
    print(
        f"  with header unit: psychicstd {pw:.1f} ms vs system {sw:.1f} ms  ({sw / pw:.2f}x)"
    )

    print("\n=== amortization: build the BMI once + compile K TUs, vs no module ===")
    for label, (base, build, withhu, size) in rows.items():
        if base > withhu:
            print(f"  {label:<12}: break-even at K = {build / (base - withhu):.1f} TUs")
        else:
            print(f"  {label:<12}: module never pays off (with-hu >= baseline)")


if __name__ == "__main__":
    main()
