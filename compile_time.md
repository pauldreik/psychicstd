# Focused Compile-Time Benchmarks

Median elapsed time and peak compiler RSS from 10 compilations per workload. Ordered by system STL compile time, slowest first.

Each psychicstd workload is measured in strict and drop-in mode; the system workload is compiled once and shared by both rows.

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`

Language mode: `C++20`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-08-14 03:56

| | name | mode | system | psychicstd | speedup | 95% CI | system peak RSS | psychicstd peak RSS |
|--|------|------|-------:|----------:|--------:|-------:|---------------:|--------------------:|
| 🟢 | all-headers | strict | 1057.5ms | 152.0ms | 6.96x | [6.90x, 6.99x] | 271.2 MiB | 68.5 MiB |
| 🟢 | all-headers | drop-in | 1057.5ms | 148.4ms | 7.13x | [7.03x, 7.15x] | 271.2 MiB | 68.5 MiB |
| 🟢 | wordcounter | strict | 376.8ms | 104.0ms | 3.62x | [3.59x, 3.70x] | 114.5 MiB | 49.7 MiB |
| 🟢 | wordcounter | drop-in | 376.8ms | 113.5ms | 3.32x | [3.24x, 3.35x] | 114.5 MiB | 52.2 MiB |
| 🟢 | iostream | strict | 181.3ms | 24.5ms | 7.39x | [7.29x, 7.50x] | 71.7 MiB | 28.9 MiB |
| 🟢 | iostream | drop-in | 181.3ms | 49.8ms | 3.64x | [3.61x, 3.69x] | 71.7 MiB | 39.1 MiB |

## Strict-mode improvement

Reduction relative to psychicstd's drop-in mode; positive values mean strict mode uses less time or memory.

| workload | compile-time reduction | 95% CI | peak-RSS reduction |
|---|---:|---:|---:|
| all-headers | -2.4% | [-3.3%, -1.5%] | 0.0% |
| wordcounter | 8.3% | [7.1%, 11.4%] | 4.7% |
| iostream | 50.7% | [50.1%, 51.3%] | 26.1% |

______________________________________________________________________

Reproduce on your machine:

```bash
scripts/benchmark_compile_time.py c++ --reps 10
```
