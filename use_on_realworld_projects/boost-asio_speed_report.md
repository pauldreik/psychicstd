# Real-world project speed comparison

Compiler: `c++ (Ubuntu 12.3.0-1ubuntu1~22.04.3) 12.3.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## boost-asio (1.91.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 28.10 | 29.13 | 🟡 0.96x [0.91x, 1.23x] | |
| compile | 3.08 | 1.92 | 🟢 1.60x [1.50x, 1.94x] | Representative upstream Asio tests are compiled and linked directly; unrelated Boost libraries are excluded. |
| run tests | 8.02 | 8.01 | 🟢 1.00x [1.00x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 28.35 | 28.19 | 🟡 1.01x [0.99x, 1.01x] | |
| compile | 3.07 | 1.84 | 🟢 1.67x [1.62x, 1.70x] | Representative upstream Asio tests are compiled and linked directly; unrelated Boost libraries are excluded. |
| run tests | 8.02 | 8.01 | 🟢 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3`
