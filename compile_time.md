# Focused Compile-Time Benchmarks

Median elapsed time and peak compiler RSS from 10 compilations per workload. Ordered by system STL compile time, slowest first.

Each psychicstd workload is measured in strict and drop-in mode; the system workload is compiled once and shared by both rows.

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`

Language mode: `C++20`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-07-27 09:30

| | name | mode | system | psychicstd | speedup | 95% CI | system peak RSS | psychicstd peak RSS |
|--|------|------|-------:|----------:|--------:|-------:|---------------:|--------------------:|
| 🟢 | all-headers | strict | 1128.3ms | 133.5ms | 8.45x | [7.72x, 8.77x] | 270.7 MiB | 61.5 MiB |
| 🟢 | all-headers | drop-in | 1128.3ms | 137.5ms | 8.20x | [7.67x, 8.44x] | 270.7 MiB | 61.5 MiB |
| 🟢 | wordcounter | strict | 422.5ms | 114.3ms | 3.69x | [3.55x, 3.94x] | 114.1 MiB | 47.3 MiB |
| 🟢 | wordcounter | drop-in | 422.5ms | 120.3ms | 3.51x | [3.30x, 3.69x] | 114.1 MiB | 49.5 MiB |
| 🟢 | iostream | strict | 198.2ms | 39.1ms | 5.07x | [4.75x, 5.28x] | 71.4 MiB | 33.9 MiB |
| 🟢 | iostream | drop-in | 198.2ms | 52.5ms | 3.77x | [3.54x, 3.95x] | 71.4 MiB | 36.8 MiB |

## Strict-mode improvement

Reduction relative to psychicstd's drop-in mode; positive values mean strict mode uses less time or memory.

| workload | compile-time reduction | 95% CI | peak-RSS reduction |
|---|---:|---:|---:|
| all-headers | 2.9% | [-6.4%, 10.5%] | 0.0% |
| wordcounter | 5.0% | [0.6%, 11.8%] | 4.6% |
| iostream | 25.7% | [20.9%, 30.4%] | 7.9% |

______________________________________________________________________

Reproduce on your machine:

```bash
scripts/benchmark_compile_time.py c++ --reps 10
```
