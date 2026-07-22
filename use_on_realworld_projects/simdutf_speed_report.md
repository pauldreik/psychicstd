# Real-world project speed comparison

Compiler: `g++-14 (Debian 14.2.0-19) 14.2.0`. Each project is built 2 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 20 jobs at 1.5 GiB/job). ccache was disabled.

## simdutf (master260712)

simdutf code is mostly simd intrinsics.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 9.56 | 5.81 | 🟢 1.64x [1.61x, 1.68x] | |
| run tests | 21.82 | 18.74 | 🟢 1.16x [1.11x, 1.22x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 19.72 | 18.76 | 🟢 1.05x [1.04x, 1.06x] | |
| run tests | 1.30 | 1.20 | 🟢 1.09x [1.06x, 1.12x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler g++-14 --build-type both --reps 2 --jobs 8`
