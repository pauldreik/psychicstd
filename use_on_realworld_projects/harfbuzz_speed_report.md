# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## harfbuzz (14.2.1)

Builds HarfBuzz's static libraries and runs its dependency-free upstream tests; optional integrations and command-line tools are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 33.51 | 25.35 | 🟢 1.32x [1.31x, 1.33x] | |
| run tests | 4.47 | 4.56 | 🔴 0.98x [0.97x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 38.26 | 31.50 | 🟢 1.21x [1.21x, 1.22x] | |
| run tests | 0.95 | 0.97 | 🟡 0.97x [0.93x, 1.04x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
