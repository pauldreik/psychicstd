# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 7 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## rdfind (commit 787b01ab378c)

rdfind is an autoconf based project. It uses psychic strict mode.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.04 | 0.91 | 🟢 1.14x [1.13x, 1.15x] | |
| compile | 0.90 | 0.24 | 🟢 3.80x [3.61x, 3.88x] | |
| run tests | 2.49 | 5.89 | 🔴 0.42x [0.42x, 0.42x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.06 | 0.93 | 🟢 1.14x [1.13x, 1.15x] | |
| compile | 1.15 | 0.47 | 🟢 2.45x [2.40x, 2.49x] | |
| run tests | 2.47 | 1.29 | 🟢 1.92x [1.89x, 1.93x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 7`
