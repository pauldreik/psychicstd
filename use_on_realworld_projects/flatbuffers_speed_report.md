# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## flatbuffers (25.12.19)

Builds the FlatBuffers compiler, library, samples, and upstream C++ test suite.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.50 | 0.50 | 🟡 1.02x [0.99x, 1.03x] | |
| compile | 16.65 | 7.21 | 🟢 2.31x [2.29x, 2.34x] | |
| run tests | 0.10 | 0.07 | 🟢 1.33x [1.32x, 1.35x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.51 | 0.50 | 🟢 1.01x [1.00x, 1.03x] | |
| compile | 29.24 | 22.95 | 🟢 1.27x [1.27x, 1.28x] | |
| run tests | 0.03 | 0.04 | 🔴 0.84x [0.82x, 0.86x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
