# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **20 jobs** (20 logical CPUs available; the memory estimate permits 22 jobs at 1.5 GiB/job). ccache was disabled.

## abseil (20260107.1)

Builds absl/base and runs eight small upstream base tests.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.46 | 1.42 | 🟡 1.03x [0.96x, 1.11x] | |
| compile | 4.24 | 1.86 | 🟢 2.28x [2.08x, 2.44x] | |
| run tests | 0.28 | 0.18 | 🟢 1.52x [1.50x, 1.60x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.40 | 1.46 | 🟡 0.96x [0.93x, 1.03x] | |
| compile | 7.42 | 5.80 | 🟢 1.28x [1.24x, 1.31x] | |
| run tests | 0.05 | 0.05 | 🔴 0.89x [0.85x, 0.92x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5 --jobs 20`
