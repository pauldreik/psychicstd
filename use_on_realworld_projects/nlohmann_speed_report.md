# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 7 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 14 jobs at 1.5 GiB/job). ccache was disabled.

## nlohmann (3.12.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.63 | 0.64 | 🟡 0.99x [0.99x, 1.00x] | |
| compile | 29.16 | 14.21 | 🟢 2.05x [2.03x, 2.06x] | |
| run tests | 25.10 | 23.23 | 🟡 1.08x [0.98x, 1.15x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 38.64 | 14.47 | 🟢 2.67x [2.65x, 2.69x] | 217 documented API examples, compiled but not run |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.63 | 0.63 | 🟡 1.00x [0.99x, 1.01x] | |
| compile | 56.50 | 48.00 | 🟢 1.18x [1.17x, 1.18x] | |
| run tests | 6.46 | 7.59 | 🔴 0.85x [0.83x, 0.87x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 38.54 | 14.33 | 🟢 2.69x [2.68x, 2.71x] | 217 documented API examples, compiled but not run |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 7 --jobs 8`
