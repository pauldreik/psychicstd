# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## ssvstart (89c77a1)

Builds and runs SSVStart's upstream input-utility assertions against its current header dependencies.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 0.90 | 0.23 | 🟢 3.82x [3.78x, 3.88x] | |
| run tests | 0.00 | 0.00 | 🟢 1.61x [1.52x, 1.77x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 0.89 | 0.24 | 🟢 3.79x [3.68x, 3.83x] | |
| run tests | 0.00 | 0.00 | 🟢 1.78x [1.55x, 1.88x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
