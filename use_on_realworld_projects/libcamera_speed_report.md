# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## libcamera (0.7.2)

Builds libcamera's core libraries and UVC pipeline handler; hardware-dependent applications, optional integrations, bindings, and tests are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 14.61 | 3.40 | 🟢 4.29x [4.20x, 4.36x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 15.04 | 9.91 | 🟢 1.52x [1.50x, 1.52x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
