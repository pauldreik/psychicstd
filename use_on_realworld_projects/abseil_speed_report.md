# Real-world project speed comparison

Compiler: `c++ (Ubuntu 12.3.0-1ubuntu1~22.04.3) 12.3.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## abseil (20260107.1)

Builds and runs Abseil's upstream absl/base tests.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 2.06 | 2.09 | 🟡 0.99x [0.92x, 1.09x] | |
| compile | 20.15 | 8.81 | 🟢 2.29x [2.22x, 2.35x] | |
| run tests | 0.44 | 0.36 | 🟢 1.21x [1.17x, 1.25x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 2.13 | 2.33 | 🟡 0.92x [0.89x, 1.13x] | |
| compile | 24.48 | 17.40 | 🟢 1.41x [1.39x, 1.42x] | |
| run tests | 0.10 | 0.10 | 🟢 1.09x [1.04x, 1.15x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3`
