# Real-world project speed comparison

Compiler: `c++ (Ubuntu 12.3.0-1ubuntu1~22.04.3) 12.3.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## rapidjson (master-24b5e7a8b27f)

RapidJSON's examples, archivertest, and unit tests are built; simpledom and unittest are run.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.56 | 0.58 | 🟡 0.96x [0.89x, 1.00x] | |
| compile | 36.75 | 27.09 | 🟢 1.36x [1.23x, 1.42x] | |
| run example | 0.00 | 0.00 | 🟢 1.61x [1.18x, 1.68x] | |
| run tests | 1.66 | 1.65 | 🟡 1.01x [0.98x, 1.07x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.57 | 0.57 | 🟡 1.00x [0.95x, 1.05x] | |
| compile | 97.46 | 95.03 | 🟢 1.03x [1.02x, 1.05x] | |
| run example | 0.00 | 0.00 | 🟢 1.62x [1.21x, 1.86x] | |
| run tests | 0.29 | 0.29 | 🟡 0.99x [0.98x, 1.02x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5`
