# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 4 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 21 jobs at 1.5 GiB/job). ccache was disabled.

## googletest (1.16.0)

Builds GoogleTest's upstream unit tests with GMock and samples disabled, then runs the resulting CTest suite.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.56 | 0.55 | 🟡 1.01x [0.94x, 1.03x] | |
| compile | 17.15 | 11.34 | 🟢 1.51x [1.51x, 1.54x] | |
| run tests | 2.06 | 2.06 | 🟡 1.00x [1.00x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.56 | 0.56 | 🟡 1.01x [1.00x, 1.02x] | |
| compile | 38.92 | 39.98 | 🔴 0.97x [0.97x, 0.98x] | |
| run tests | 2.06 | 2.06 | 🟡 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 4 --jobs 8`
