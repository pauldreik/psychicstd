# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 14 jobs at 1.5 GiB/job). ccache was disabled.

## rdfind (commit cac59ade85de)

rdfind is an autoconf based project. It uses psychic strict mode.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.10 | 0.97 | 🟢 1.13x [1.11x, 1.17x] | |
| compile | 0.90 | 0.22 | 🟢 4.18x [4.10x, 4.28x] | |
| run tests | 2.56 | 1.41 | 🟢 1.81x [1.79x, 1.89x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.12 | 1.00 | 🟢 1.12x [1.10x, 1.16x] | |
| compile | 1.13 | 0.45 | 🟢 2.53x [2.49x, 2.56x] | |
| run tests | 2.57 | 1.41 | 🟢 1.82x [1.78x, 1.91x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
