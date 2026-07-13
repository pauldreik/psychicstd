# Real-world project speed comparison

Compiler: `g++-14 (Debian 14.3.0-14) 14.3.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## catch2 (3.8.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 11.27 | 3.46 | 🟢 3.26x [3.24x, 3.35x] | |
| run tests | 2.00 | 2.87 | 🟡 0.70x [0.69x, 1.28x] | the approval tests are ignored |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 14.59 | 8.03 | 🟢 1.82x [1.80x, 1.83x] | |
| run tests | 1.50 | 1.49 | 🟡 1.01x [1.00x, 1.02x] | the approval tests are ignored |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler g++-14 --build-type both --reps 9`
