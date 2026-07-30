# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 21 jobs at 1.5 GiB/job). ccache was disabled.

## cmake (4.3.4)

Builds upstream CMake's core static library together with its KWSys, std-compatibility, and JSON support targets, then runs the supported KWSys tests. OpenSSL and debugger support are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 20.92 | 18.51 | 🟢 1.13x [1.11x, 1.16x] | |
| compile | 94.30 | 31.02 | 🟢 3.04x [3.03x, 3.09x] | |
| run tests | 1.18 | 1.17 | 🟢 1.00x [1.00x, 1.01x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 21.16 | 18.57 | 🟢 1.14x [1.12x, 1.17x] | |
| compile | 98.66 | 52.56 | 🟢 1.88x [1.87x, 1.89x] | |
| run tests | 1.17 | 1.17 | 🟢 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 5 --jobs 8`
