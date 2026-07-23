# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 18 jobs at 1.5 GiB/job). ccache was disabled.

## cmake (4.3.4)

Builds upstream CMake's core static library together with its KWSys, std-compatibility, and JSON support targets, then runs the supported KWSys tests. OpenSSL and debugger support are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 21.19 | 18.22 | 🟢 1.16x [1.16x, 1.17x] | |
| compile | 94.79 | 29.06 | 🟢 3.26x [3.22x, 3.29x] | |
| run tests | 1.17 | 1.17 | 🟢 1.00x [1.00x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 20.89 | 18.64 | 🟢 1.12x [1.11x, 1.17x] | |
| compile | 99.21 | 51.46 | 🟢 1.93x [1.92x, 1.93x] | |
| run tests | 1.17 | 1.17 | 🟢 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5 --jobs 8`
