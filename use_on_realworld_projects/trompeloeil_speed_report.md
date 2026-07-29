# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 22 jobs at 1.5 GiB/job). ccache was disabled.

## trompeloeil (v49)

Builds Trompeloeil's complete self-test suite and runs its coroutine, threaded, and custom-mutex tests. The remaining self-tests rely on ECMAScript regex behavior beyond psychicstd's POSIX-backed implementation.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.39 | 0.38 | 🟡 1.02x [0.96x, 1.03x] | |
| compile | 23.65 | 14.83 | 🟢 1.59x [1.59x, 1.60x] | |
| run tests | 1.80 | 1.51 | 🟢 1.19x [1.15x, 1.31x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.39 | 0.38 | 🟡 1.02x [0.98x, 1.02x] | |
| compile | 41.05 | 38.42 | 🟢 1.07x [1.06x, 1.08x] | |
| run tests | 0.70 | 0.57 | 🟢 1.23x [1.13x, 1.29x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 3 --jobs 8`
