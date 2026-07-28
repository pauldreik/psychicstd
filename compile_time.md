# Focused Compile-Time Benchmarks

Median elapsed time and peak compiler RSS from 10 compilations per workload. Ordered by system STL compile time, slowest first.

Each psychicstd workload is measured in strict and drop-in mode; the system workload is compiled once and shared by both rows.

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`

Language mode: `C++20`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-07-28 03:55

| | name | mode | system | psychicstd | speedup | 95% CI | system peak RSS | psychicstd peak RSS |
|--|------|------|-------:|----------:|--------:|-------:|---------------:|--------------------:|
| 🟢 | all-headers | strict | 1014.8ms | 122.0ms | 8.32x | [8.28x, 8.36x] | 271.2 MiB | 61.7 MiB |
| 🟢 | all-headers | drop-in | 1014.8ms | 121.4ms | 8.36x | [8.32x, 8.40x] | 271.2 MiB | 61.8 MiB |
| 🟢 | wordcounter | strict | 372.3ms | 100.2ms | 3.71x | [3.70x, 3.73x] | 114.5 MiB | 47.5 MiB |
| 🟢 | wordcounter | drop-in | 372.3ms | 111.6ms | 3.34x | [3.33x, 3.35x] | 114.5 MiB | 51.9 MiB |
| 🟢 | iostream | strict | 173.5ms | 36.9ms | 4.71x | [4.64x, 4.78x] | 71.6 MiB | 34.1 MiB |
| 🟢 | iostream | drop-in | 173.5ms | 48.1ms | 3.61x | [3.59x, 3.65x] | 71.6 MiB | 36.9 MiB |

## Strict-mode improvement

Reduction relative to psychicstd's drop-in mode; positive values mean strict mode uses less time or memory.

| workload | compile-time reduction | 95% CI | peak-RSS reduction |
|---|---:|---:|---:|
| all-headers | -0.5% | [-0.8%, -0.1%] | 0.1% |
| wordcounter | 10.2% | [9.9%, 10.5%] | 8.3% |
| iostream | 23.4% | [22.2%, 24.2%] | 7.6% |

______________________________________________________________________

Reproduce on your machine:

```bash
scripts/benchmark_compile_time.py c++ --reps 10
```
