# Real-world project speed comparison

Compiler: `g++-14 (Debian 14.2.0-19) 14.2.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 20 jobs at 1.5 GiB/job). ccache was disabled.

## eigen (3.4.0)

eigen has no configure step; a fixed subset of its test suite is compiled and run individually, with times summed.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 20.69 | 10.72 | 🟢 1.93x [1.90x, 1.94x] | |
| run tests | 0.03 | 0.03 | 🟢 1.29x [1.22x, 1.39x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 20.58 | 10.81 | 🟢 1.90x [1.89x, 1.93x] | |
| run tests | 0.03 | 0.03 | 🟢 1.33x [1.29x, 1.39x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler g++-14 --build-type both --reps 3 --jobs 8`
