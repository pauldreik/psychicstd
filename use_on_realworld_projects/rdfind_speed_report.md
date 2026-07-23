# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 18 jobs at 1.5 GiB/job). ccache was disabled.

## rdfind (commit 787b01ab378c)

rdfind is an autoconf based project. It uses psychic strict mode.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.08 | 0.96 | 🟢 1.13x [1.10x, 1.14x] | |
| compile | 0.90 | 0.22 | 🟢 4.16x [4.08x, 4.20x] | |
| run tests | 2.55 | 1.38 | 🟢 1.84x [1.81x, 1.88x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.11 | 0.95 | 🟢 1.17x [1.12x, 1.18x] | |
| compile | 1.16 | 0.73 | 🟢 1.58x [1.55x, 1.59x] | |
| run tests | 2.53 | 1.39 | 🟢 1.82x [1.80x, 1.89x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 9 --jobs 8`
