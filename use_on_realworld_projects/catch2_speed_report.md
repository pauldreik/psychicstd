# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 18 jobs at 1.5 GiB/job). ccache was disabled.

## catch2 (3.8.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 16.03 | 4.42 | 🟢 3.63x [3.57x, 3.67x] | |
| run tests | 1.98 | 1.63 | 🟢 1.22x [1.18x, 1.23x] | the approval tests are ignored |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 20.17 | 13.00 | 🟢 1.55x [1.54x, 1.56x] | |
| run tests | 1.53 | 1.54 | 🟡 1.00x [0.98x, 1.00x] | the approval tests are ignored |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 9 --jobs 8`
