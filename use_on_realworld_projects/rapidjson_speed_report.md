# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 18 jobs at 1.5 GiB/job). ccache was disabled.

## rapidjson (master-24b5e7a8b27f)

RapidJSON's examples, archivertest, and unit tests are built; simpledom and unittest are run.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.44 | 0.43 | 🟡 1.00x [1.00x, 1.01x] | |
| compile | 11.41 | 9.06 | 🟢 1.26x [1.24x, 1.26x] | |
| run example | 0.00 | 0.00 | 🟢 1.76x [1.32x, 2.01x] | |
| run tests | 2.22 | 2.24 | 🟡 0.99x [0.98x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.43 | 0.44 | 🟡 0.99x [0.98x, 1.00x] | |
| compile | 44.31 | 45.46 | 🔴 0.97x [0.97x, 0.98x] | |
| run example | 0.00 | 0.00 | 🟢 1.67x [1.53x, 1.84x] | |
| run tests | 0.34 | 0.33 | 🟢 1.02x [1.01x, 1.03x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5 --jobs 8`
