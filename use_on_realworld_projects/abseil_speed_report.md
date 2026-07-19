# Real-world project speed comparison

Compiler: `c++ (Ubuntu 12.3.0-1ubuntu1~22.04.3) 12.3.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## abseil (20260107.1)

Builds and runs Abseil's upstream absl/base tests.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 2.63 | 2.41 | 🟢 1.09x [1.04x, 1.20x] | |
| compile | 15.33 | 7.14 | 🟢 2.15x [2.11x, 2.37x] | |
| run tests | 0.26 | 0.25 | 🟢 1.04x [1.01x, 1.11x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 2.45 | 2.45 | 🟡 1.00x [0.90x, 1.55x] | |
| compile | 19.67 | 14.23 | 🟢 1.38x [1.31x, 1.41x] | |
| run tests | 0.09 | 0.08 | 🟡 1.07x [0.99x, 1.16x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3`
