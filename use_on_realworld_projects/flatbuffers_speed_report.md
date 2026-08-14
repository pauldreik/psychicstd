# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 17 jobs at 1.5 GiB/job). ccache was disabled.

## flatbuffers (25.12.19)

Builds the FlatBuffers compiler, library, samples, and upstream C++ test suite.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.50 | 0.50 | 🟡 1.01x [0.99x, 1.02x] | |
| compile | 17.08 | 7.39 | 🟢 2.31x [2.28x, 2.50x] | |
| run tests | 0.10 | 0.08 | 🟢 1.24x [1.21x, 1.25x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.51 | 0.51 | 🟡 1.01x [0.99x, 1.02x] | |
| compile | 29.73 | 23.27 | 🟢 1.28x [1.27x, 1.29x] | |
| run tests | 0.03 | 0.04 | 🔴 0.86x [0.83x, 0.87x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
