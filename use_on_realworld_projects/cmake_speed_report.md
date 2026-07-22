# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 21 jobs at 1.5 GiB/job). ccache was disabled.

## cmake (4.3.4)

Builds upstream CMake's core static library together with its KWSys, std-compatibility, and JSON support targets, then runs the supported KWSys tests. OpenSSL and debugger support are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 21.26 | 18.41 | 🟢 1.15x [1.14x, 1.18x] | |
| compile | 94.44 | 30.62 | 🟢 3.08x [2.99x, 3.11x] | |
| run tests | 1.18 | 1.17 | 🟢 1.01x [1.00x, 1.01x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 20.83 | 18.21 | 🟢 1.14x [1.12x, 1.18x] | |
| compile | 98.97 | 51.40 | 🟢 1.93x [1.91x, 1.94x] | |
| run tests | 1.17 | 1.17 | 🟢 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3 --jobs 8`
