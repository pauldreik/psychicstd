# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 7 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## nlohmann (3.12.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.61 | 0.61 | 🟡 1.01x [0.99x, 1.02x] | |
| compile | 19.23 | 9.76 | 🟢 1.97x [1.95x, 2.01x] | |
| run tests | 25.37 | 23.43 | 🟢 1.08x [1.06x, 1.11x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 24.63 | 9.62 | 🟢 2.56x [2.55x, 2.57x] | 217 documented API examples, compiled but not run |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.61 | 0.61 | 🟡 1.00x [0.99x, 1.01x] | |
| compile | 38.22 | 32.35 | 🟢 1.18x [1.17x, 1.19x] | |
| run tests | 6.43 | 6.84 | 🟡 0.94x [0.86x, 1.07x] | unicode/cbor/msgpack (slow), algorithms (unspecified tail order), cmake_fetch/cmake_import (not applicable) excluded |
| examples | 24.61 | 9.63 | 🟢 2.56x [2.55x, 2.56x] | 217 documented API examples, compiled but not run |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 7`
