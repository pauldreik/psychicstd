# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 21 jobs at 1.5 GiB/job). ccache was disabled.

## rdfind (commit 787b01ab378c)

rdfind is an autoconf based project. It uses psychic strict mode.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.05 | 0.92 | 🟢 1.14x [1.10x, 1.15x] | |
| compile | 0.90 | 0.27 | 🟢 3.33x [3.28x, 3.43x] | |
| run tests | 2.52 | 1.33 | 🟢 1.90x [1.87x, 1.90x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.08 | 0.95 | 🟢 1.13x [1.11x, 1.14x] | |
| compile | 1.15 | 0.58 | 🟢 1.97x [1.96x, 2.00x] | |
| run tests | 2.51 | 1.34 | 🟢 1.88x [1.81x, 1.91x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5 --jobs 8`
