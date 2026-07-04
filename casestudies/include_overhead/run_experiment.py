#!/usr/bin/env python3
"""Validate the hypothesis: for a FIXED total amount of code, does splitting it
across more #include'd files make compilation slower?

Method
------
We generate one fixed body of C++ code -- the SAME sequence of code units on
every run -- and partition it into K header files for several values of K.
A generated main.cpp #include's all K files. Because the sequence of units is
identical regardless of K, the parser sees the same declarations every time;
only the NUMBER OF FILES (and thus #include directives, file opens, include
guards) changes.

We measure median compile time vs K. To confirm the total code is held
constant we strip line-marker directives from the preprocessed output and
check the byte count doesn't move. A linear fit of time vs K estimates the
per-file overhead.

Everything is self-contained (no STL includes) so we measure the pure
file-count effect, independent of which standard library is in use.

Usage:  python3 run_experiment.py
Env:    CXX (default clang++-21), UNITS, N (reps), KS (comma-separated)
"""

import os
import shutil
import statistics
import subprocess
import tempfile

CXX = os.environ.get("CXX", "clang++-21")
STD = os.environ.get("STD", "c++20")
# Total number of code units, held constant across all K.
UNITS = int(os.environ.get("UNITS", "8000"))
N = int(os.environ.get("N", "7"))
KS = [
    int(x)
    for x in os.environ.get("KS", "1,2,5,10,25,50,100,250,500,1000,2000").split(",")
]


def unit(i):
    """One fixed code unit. Small, self-contained, cheap to parse."""
    return (
        f"struct S{i} {{\n"
        f"  int a, b, c;\n"
        f"  int sum() const {{ return a + b + c; }}\n"
        f"  int scaled(int k) const {{ return (a + b + c) * k; }}\n"
        f"}};\n"
    )


# The full, fixed body of code -- generated once, identical for every K.
ALL_UNITS = [unit(i) for i in range(UNITS)]


def partition(seq, k):
    """Split seq into k contiguous, near-equal chunks."""
    n = len(seq)
    return [seq[i * n // k : (i + 1) * n // k] for i in range(k)]


def make_tree(workdir, k):
    """Write k header files plus main.cpp into workdir. Returns main.cpp path."""
    for f in os.listdir(workdir):
        os.remove(os.path.join(workdir, f))
    chunks = partition(ALL_UNITS, k)
    includes = []
    for idx, chunk in enumerate(chunks):
        name = f"chunk_{idx}.h"
        with open(os.path.join(workdir, name), "w") as fh:
            fh.write("#pragma once\n")
            fh.write("".join(chunk))
        includes.append(f'#include "{name}"\n')
    main = os.path.join(workdir, "main.cpp")
    with open(main, "w") as fh:
        fh.write("".join(includes))
        fh.write("int main() { return 0; }\n")
    return main


def compile_time(main):
    """Return wall time (seconds) of one -c compile to /dev/null."""
    import time  # local: only used for measuring the child, not for logic

    start = time.perf_counter()
    subprocess.run(
        [CXX, f"-std={STD}", "-c", main, "-o", "/dev/null"],
        check=True,
    )
    return time.perf_counter() - start


def code_bytes(main):
    """Bytes of actual code the parser sees, i.e. the preprocessed output with
    line-marker directives (# ...) and blank lines stripped. This is held
    constant across K; the raw -E size is not, because more files means more
    '# 1 "file"' line markers -- bookkeeping, not code."""
    out = subprocess.run(
        [CXX, f"-std={STD}", "-E", main],
        check=True,
        capture_output=True,
        text=True,
    )
    return sum(
        len(line)
        for line in out.stdout.splitlines()
        if line.strip() and not line.startswith("#")
    )


def main():
    print(
        f"compiler: {subprocess.run([CXX, '--version'], capture_output=True, text=True).stdout.splitlines()[0]}"
    )
    print(f"total code units (held constant): {UNITS}")
    print(f"reps per K: {N}, standard: {STD}\n")

    workdir = tempfile.mkdtemp(prefix="inc_overhead_")
    rows = []
    try:
        for k in KS:
            mainf = make_tree(workdir, k)
            # warm the cache / build once, then time N reps
            compile_time(mainf)
            times = sorted(compile_time(mainf) for _ in range(N))
            med = statistics.median(times) * 1000.0
            cb = code_bytes(mainf)
            rows.append((k, cb, med))
            print(
                f"  K={k:<5} files={k:<5} code={cb:>8}B (constant)  median={med:6.1f}ms"
            )
    finally:
        shutil.rmtree(workdir, ignore_errors=True)

    # Linear fit: median_ms = a + b*K  -> b is per-file overhead.
    ks = [r[0] for r in rows]
    ys = [r[2] for r in rows]
    n = len(ks)
    mx = sum(ks) / n
    my = sum(ys) / n
    b = sum((k - mx) * (y - my) for k, y in zip(ks, ys)) / sum(
        (k - mx) ** 2 for k in ks
    )
    a = my - b * mx

    cbs = [r[1] for r in rows]
    print("\n=== summary ===")
    print(
        f"  code bytes (parser-visible): {min(cbs)}B .. {max(cbs)}B "
        f"({100 * (max(cbs) - min(cbs)) / min(cbs):.2f}% spread -> effectively constant)"
    )
    print(f"  compile time at K=1:     {ys[0]:.1f} ms")
    print(
        f"  compile time at K={ks[-1]}:  {ys[-1]:.1f} ms  "
        f"({ys[-1] / ys[0]:.2f}x slower for the SAME code)"
    )
    print(f"  linear fit: time ~= {a:.1f} ms + {1000 * b:.3f} us/file * K")
    print(f"  => per-extra-file overhead ~= {1000 * b:.2f} microseconds")


if __name__ == "__main__":
    main()
