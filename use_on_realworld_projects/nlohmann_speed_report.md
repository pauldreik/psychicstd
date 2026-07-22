# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **20 jobs** (20 logical CPUs available; the memory estimate permits 21 jobs at 1.5 GiB/job). ccache was disabled.

## nlohmann (3.12.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.63 | 0.63 | 🟡 1.00x [1.00x, 1.01x] | |
| compile | 15.63 | 8.44 | 🟢 1.85x [1.78x, 1.87x] | |
| run tests | 25.20 | 22.40 | 🟡 1.12x [0.74x, 1.14x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 17.61 | 7.17 | 🟢 2.46x [2.43x, 2.51x] | 217 documented API examples, compiled but not run |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.63 | 0.63 | 🟡 0.99x [0.99x, 1.01x] | |
| compile | 30.41 | 27.06 | 🟢 1.12x [1.09x, 1.15x] | |
| run tests | 7.11 | 7.25 | 🟡 0.98x [0.90x, 1.26x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 17.73 | 7.18 | 🟢 2.47x [2.47x, 2.54x] | 217 documented API examples, compiled but not run |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3`
