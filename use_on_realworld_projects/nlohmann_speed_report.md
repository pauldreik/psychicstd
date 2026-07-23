# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 18 jobs at 1.5 GiB/job). ccache was disabled.

## nlohmann (3.12.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.63 | 0.62 | 🟢 1.01x [1.00x, 1.02x] | |
| compile | 28.30 | 13.82 | 🟢 2.05x [2.04x, 2.07x] | |
| run tests | 25.89 | 22.62 | 🟡 1.14x [0.95x, 1.25x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 37.97 | 13.99 | 🟢 2.71x [2.68x, 2.77x] | 217 documented API examples, compiled but not run |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.63 | 0.62 | 🟢 1.01x [1.00x, 1.02x] | |
| compile | 54.50 | 46.29 | 🟢 1.18x [1.17x, 1.19x] | |
| run tests | 6.79 | 7.66 | 🔴 0.89x [0.66x, 0.96x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 37.87 | 13.94 | 🟢 2.72x [2.71x, 2.76x] | 217 documented API examples, compiled but not run |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5 --jobs 8`
