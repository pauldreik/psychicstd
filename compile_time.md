# Focused Compile-Time Benchmarks

Median elapsed time and peak compiler RSS from 10 compilations per workload. Ordered by system STL compile time, slowest first.

Each psychicstd workload is measured in strict and drop-in mode; the system workload is compiled once and shared by both rows.

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`

Language mode: `C++20`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-08-07 03:06

| | name | mode | system | psychicstd | speedup | 95% CI | system peak RSS | psychicstd peak RSS |
|--|------|------|-------:|----------:|--------:|-------:|---------------:|--------------------:|
| 🟢 | all-headers | strict | 1023.4ms | 138.6ms | 7.38x | [7.31x, 7.43x] | 271.2 MiB | 66.5 MiB |
| 🟢 | all-headers | drop-in | 1023.4ms | 138.7ms | 7.38x | [7.27x, 7.41x] | 271.2 MiB | 66.5 MiB |
| 🟢 | wordcounter | strict | 372.7ms | 102.8ms | 3.63x | [3.60x, 3.65x] | 114.5 MiB | 49.7 MiB |
| 🟢 | wordcounter | drop-in | 372.7ms | 112.9ms | 3.30x | [3.27x, 3.33x] | 114.5 MiB | 52.0 MiB |
| 🟢 | iostream | strict | 173.6ms | 25.0ms | 6.95x | [6.70x, 7.01x] | 71.6 MiB | 28.9 MiB |
| 🟢 | iostream | drop-in | 173.6ms | 49.0ms | 3.54x | [3.52x, 3.56x] | 71.6 MiB | 39.2 MiB |

## Strict-mode improvement

Reduction relative to psychicstd's drop-in mode; positive values mean strict mode uses less time or memory.

| workload | compile-time reduction | 95% CI | peak-RSS reduction |
|---|---:|---:|---:|
| all-headers | 0.1% | [-0.7%, 1.5%] | 0.0% |
| wordcounter | 8.9% | [8.2%, 9.6%] | 4.5% |
| iostream | 49.1% | [47.2%, 49.5%] | 26.1% |

______________________________________________________________________

Reproduce on your machine:

```bash
scripts/benchmark_compile_time.py c++ --reps 10
```
