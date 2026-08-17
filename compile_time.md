# Focused Compile-Time Benchmarks

Median elapsed time and peak compiler RSS from 10 compilations per workload. Ordered by system STL compile time, slowest first.

Each psychicstd workload is measured in strict and drop-in mode; the system workload is compiled once and shared by both rows.

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`

Language mode: `C++20`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-08-17 08:26

| | name | mode | system | psychicstd | speedup | 95% CI | system peak RSS | psychicstd peak RSS |
|--|------|------|-------:|----------:|--------:|-------:|---------------:|--------------------:|
| 🟢 | all-headers | strict | 1099.8ms | 152.2ms | 7.23x | [7.12x, 7.31x] | 271.2 MiB | 67.9 MiB |
| 🟢 | all-headers | drop-in | 1099.8ms | 156.1ms | 7.04x | [6.98x, 7.10x] | 271.2 MiB | 68.0 MiB |
| 🟢 | wordcounter | strict | 398.5ms | 100.3ms | 3.97x | [3.93x, 4.00x] | 114.5 MiB | 47.9 MiB |
| 🟢 | wordcounter | drop-in | 398.5ms | 111.7ms | 3.57x | [3.51x, 3.61x] | 114.5 MiB | 52.3 MiB |
| 🟢 | iostream | strict | 184.1ms | 23.6ms | 7.79x | [7.57x, 7.90x] | 71.7 MiB | 28.7 MiB |
| 🟢 | iostream | drop-in | 184.1ms | 50.1ms | 3.67x | [3.60x, 3.72x] | 71.7 MiB | 39.1 MiB |

## Strict-mode improvement

Reduction relative to psychicstd's drop-in mode; positive values mean strict mode uses less time or memory.

| workload | compile-time reduction | 95% CI | peak-RSS reduction |
|---|---:|---:|---:|
| all-headers | 2.5% | [1.0%, 3.8%] | 0.1% |
| wordcounter | 10.2% | [8.9%, 11.6%] | 8.5% |
| iostream | 52.8% | [51.8%, 53.5%] | 26.5% |

______________________________________________________________________

Reproduce on your machine:

```bash
scripts/benchmark_compile_time.py c++ --reps 10
```
