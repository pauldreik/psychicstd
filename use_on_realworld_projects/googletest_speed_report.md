# Real-world project speed comparison

Compiler: `c++ (Ubuntu 12.3.0-1ubuntu1~22.04.3) 12.3.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## googletest (1.16.0)

Builds GoogleTest's upstream unit tests with GMock and samples disabled, then runs the resulting CTest suite.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.70 | 1.67 | 🟡 1.02x [0.99x, 1.06x] | |
| compile | 34.37 | 20.47 | 🟢 1.68x [1.44x, 1.73x] | |
| run tests | 2.69 | 2.66 | 🟡 1.01x [0.98x, 1.03x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.69 | 1.68 | 🟡 1.01x [0.98x, 1.04x] | |
| compile | 69.18 | 61.02 | 🟢 1.13x [1.11x, 1.15x] | |
| run tests | 2.68 | 2.66 | 🟡 1.01x [0.97x, 1.04x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3`
