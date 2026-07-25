# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 18 jobs at 1.5 GiB/job). ccache was disabled.

## nlohmann (3.12.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.62 | 0.62 | 🟡 1.00x [0.99x, 1.02x] | |
| compile | 27.44 | 13.60 | 🟢 2.02x [1.99x, 2.04x] | |
| run tests | 25.39 | 22.76 | 🟢 1.12x [1.07x, 1.16x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 36.98 | 13.98 | 🟢 2.65x [2.57x, 2.69x] | 217 documented API examples, compiled but not run |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type debug --reps 5 --jobs 8`
