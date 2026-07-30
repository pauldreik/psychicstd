# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 21 jobs at 1.5 GiB/job). ccache was disabled.

## trompeloeil (v49)

Builds and runs Trompeloeil's complete self-test suite, including its coroutine, threaded, and custom-mutex tests.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.39 | 0.38 | 🟢 1.04x [1.03x, 1.04x] | |
| compile | 22.53 | 14.15 | 🟢 1.59x [1.59x, 1.60x] | |
| run tests | 1.76 | 1.46 | 🟢 1.20x [1.18x, 1.22x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.39 | 0.38 | 🟢 1.04x [1.04x, 1.05x] | |
| compile | 38.80 | 36.02 | 🟢 1.08x [1.07x, 1.08x] | |
| run tests | 0.67 | 0.59 | 🟢 1.14x [1.09x, 1.20x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
