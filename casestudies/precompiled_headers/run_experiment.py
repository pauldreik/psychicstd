#!/usr/bin/env python3
"""Measure how a precompiled header (PCH) affects compile time of a <string>
using translation unit, for both psychicstd and the system libstdc++.

For each standard library we measure three things:
  baseline  - compile tu.cpp normally (parses + instantiates <string> anew)
  pch-build - one-time cost of building the PCH from pch.h
  with-pch  - compile tu.cpp with -include-pch (reuses the cached header)

We then ask:
  - how much does PCH speed up a single TU, per config?
  - does psychicstd still beat libstdc++ once both use PCH?
  - how many TUs must you compile before the one-time PCH build pays off?

Scoped to <string> only so the psychicstd-vs-libstdc++ comparison is fair:
<string> is the most complete part of psychicstd.

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
    base = [CXX, *flags, f"-std={STD}"]
    tu = os.path.join(HERE, "tu.cpp")
    pchsrc = os.path.join(HERE, "pch.h")
    pch = os.path.join(workdir, f"{label}.pch")

    baseline = median_ms([*base, "-c", tu, "-o", "/dev/null"])
    build = median_ms([*base, "-x", "c++-header", pchsrc, "-o", pch])
    subprocess.run([*base, "-x", "c++-header", pchsrc, "-o", pch], check=True)
    size = os.path.getsize(pch)
    withpch = median_ms([*base, "-include-pch", pch, "-c", tu, "-o", "/dev/null"])
    return baseline, build, withpch, size


def main():
    ver = subprocess.run(
        [CXX, "--version"], capture_output=True, text=True
    ).stdout.splitlines()[0]
    print(f"compiler: {ver}")
    print(f"standard: {STD}, reps: {N}\n")

    workdir = tempfile.mkdtemp(prefix="pch_")
    rows = {label: measure(label, flags, workdir) for label, flags in CONFIGS.items()}

    print("=== per-config results (ms) ===")
    print(
        f"  {'config':<12}{'baseline':>10}{'pch-build':>11}{'with-pch':>10}{'pch size':>12}"
    )
    for label, (base, build, withpch, size) in rows.items():
        print(
            f"  {label:<12}{base:>10.1f}{build:>11.1f}{withpch:>10.1f}{size / 1024:>10.0f}KB"
        )

    print("\n=== does PCH help each config? ===")
    for label, (base, build, withpch, size) in rows.items():
        print(
            f"  {label:<12}: {base:.1f} -> {withpch:.1f} ms per TU  ({base / withpch:.2f}x faster)"
        )

    print("\n=== does psychicstd still win with PCH? ===")
    sb, _, sw, _ = rows["system"]
    pb, _, pw, _ = rows["psychicstd"]
    print(
        f"  baseline : psychicstd {pb:.1f} ms vs system {sb:.1f} ms  ({sb / pb:.2f}x)"
    )
    print(
        f"  with PCH : psychicstd {pw:.1f} ms vs system {sw:.1f} ms  ({sw / pw:.2f}x)"
    )

    print("\n=== amortization: build PCH once + compile K TUs, vs no PCH ===")
    for label, (base, build, withpch, size) in rows.items():
        if base > withpch:
            be = build / (base - withpch)
            print(f"  {label:<12}: break-even at K = {be:.1f} TUs")
        else:
            print(f"  {label:<12}: PCH never pays off (with-pch >= baseline)")


if __name__ == "__main__":
    main()
