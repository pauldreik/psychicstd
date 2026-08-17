# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## rdfind (commit cac59ade85de)

rdfind is an autoconf based project. It uses psychic strict mode.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.10 | 0.99 | 🟢 1.12x [1.10x, 1.14x] | |
| compile | 0.92 | 0.22 | 🟢 4.13x [3.92x, 4.26x] | |
| run tests | 2.58 | 1.40 | 🟢 1.84x [1.81x, 1.88x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.13 | 0.99 | 🟢 1.14x [1.10x, 1.15x] | |
| compile | 1.14 | 0.41 | 🟢 2.78x [2.67x, 2.89x] | |
| run tests | 2.55 | 1.39 | 🟢 1.84x [1.81x, 1.88x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
