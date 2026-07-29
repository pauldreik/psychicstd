# Focused Compile-Time Benchmarks

Median elapsed time and peak compiler RSS from 10 compilations per workload. Ordered by system STL compile time, slowest first.

Each psychicstd workload is measured in strict and drop-in mode; the system workload is compiled once and shared by both rows.

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`

Language mode: `C++20`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-07-29 17:38

| | name | mode | system | psychicstd | speedup | 95% CI | system peak RSS | psychicstd peak RSS |
|--|------|------|-------:|----------:|--------:|-------:|---------------:|--------------------:|
| 🟢 | all-headers | strict | 1007.6ms | 126.3ms | 7.98x | [7.93x, 8.01x] | 271.2 MiB | 63.9 MiB |
| 🟢 | all-headers | drop-in | 1007.6ms | 126.3ms | 7.98x | [7.92x, 8.02x] | 271.2 MiB | 63.9 MiB |
| 🟢 | wordcounter | strict | 368.3ms | 99.5ms | 3.70x | [3.68x, 3.73x] | 114.5 MiB | 47.5 MiB |
| 🟢 | wordcounter | drop-in | 368.3ms | 111.5ms | 3.30x | [3.29x, 3.33x] | 114.5 MiB | 51.9 MiB |
| 🟢 | iostream | strict | 172.9ms | 24.5ms | 7.04x | [6.98x, 7.15x] | 71.6 MiB | 28.9 MiB |
| 🟢 | iostream | drop-in | 172.9ms | 48.1ms | 3.59x | [3.57x, 3.64x] | 71.6 MiB | 37.1 MiB |

## Strict-mode improvement

Reduction relative to psychicstd's drop-in mode; positive values mean strict mode uses less time or memory.

| workload | compile-time reduction | 95% CI | peak-RSS reduction |
|---|---:|---:|---:|
| all-headers | 0.0% | [-0.4%, 0.4%] | 0.1% |
| wordcounter | 10.7% | [10.3%, 11.1%] | 8.5% |
| iostream | 49.0% | [48.5%, 49.5%] | 22.1% |

______________________________________________________________________

Reproduce on your machine:

```bash
scripts/benchmark_compile_time.py c++ --reps 10
```
