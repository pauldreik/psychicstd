# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## abseil (20260107.1)

Builds absl/base and runs eight small upstream base tests.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.30 | 1.31 | 🟡 1.00x [0.98x, 1.04x] | |
| compile | 3.86 | 1.76 | 🟢 2.19x [2.16x, 2.25x] | |
| run tests | 0.28 | 0.17 | 🟢 1.62x [1.59x, 1.62x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.27 | 1.27 | 🟡 1.00x [0.99x, 1.01x] | |
| compile | 6.75 | 5.41 | 🟢 1.25x [1.24x, 1.25x] | |
| run tests | 0.04 | 0.06 | 🔴 0.78x [0.77x, 0.80x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
