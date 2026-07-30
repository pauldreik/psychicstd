# Focused Compile-Time Benchmarks

Median elapsed time and peak compiler RSS from 10 compilations per workload. Ordered by system STL compile time, slowest first.

Each psychicstd workload is measured in strict and drop-in mode; the system workload is compiled once and shared by both rows.

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`

Language mode: `C++20`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-07-30 06:21

| | name | mode | system | psychicstd | speedup | 95% CI | system peak RSS | psychicstd peak RSS |
|--|------|------|-------:|----------:|--------:|-------:|---------------:|--------------------:|
| 🟢 | all-headers | strict | 1012.1ms | 128.0ms | 7.91x | [7.87x, 7.93x] | 271.2 MiB | 64.1 MiB |
| 🟢 | all-headers | drop-in | 1012.1ms | 128.2ms | 7.89x | [7.88x, 7.92x] | 271.2 MiB | 64.1 MiB |
| 🟢 | wordcounter | strict | 369.6ms | 99.7ms | 3.71x | [3.70x, 3.72x] | 114.6 MiB | 47.5 MiB |
| 🟢 | wordcounter | drop-in | 369.6ms | 111.9ms | 3.30x | [3.29x, 3.31x] | 114.6 MiB | 52.0 MiB |
| 🟢 | iostream | strict | 173.3ms | 24.5ms | 7.08x | [7.00x, 7.12x] | 71.6 MiB | 28.9 MiB |
| 🟢 | iostream | drop-in | 173.3ms | 48.0ms | 3.61x | [3.60x, 3.62x] | 71.6 MiB | 37.2 MiB |

## Strict-mode improvement

Reduction relative to psychicstd's drop-in mode; positive values mean strict mode uses less time or memory.

| workload | compile-time reduction | 95% CI | peak-RSS reduction |
|---|---:|---:|---:|
| all-headers | 0.2% | [-0.2%, 0.3%] | -0.0% |
| wordcounter | 10.9% | [10.7%, 11.2%] | 8.7% |
| iostream | 49.0% | [48.5%, 49.3%] | 22.3% |

______________________________________________________________________

Reproduce on your machine:

```bash
scripts/benchmark_compile_time.py c++ --reps 10
```
