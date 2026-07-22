# Real-world project speed comparison

Compiler: `g++-14 (Debian 14.2.0-19) 14.2.0`. Each project is built 4 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 20 jobs at 1.5 GiB/job). ccache was disabled.

## catch2 (3.8.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 16.19 | 4.53 | 🟢 3.58x [3.43x, 3.71x] | |
| run tests | 2.00 | 1.67 | 🟢 1.20x [1.10x, 1.22x] | the approval tests are ignored |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 20.58 | 13.17 | 🟢 1.56x [1.54x, 1.58x] | |
| run tests | 1.55 | 1.57 | 🟡 0.99x [0.97x, 1.02x] | the approval tests are ignored |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler g++-14 --build-type both --reps 4 --jobs 8`
