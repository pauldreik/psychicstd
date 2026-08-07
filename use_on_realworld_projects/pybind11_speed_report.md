# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 17 jobs at 1.5 GiB/job). ccache was disabled.

## pybind11 (3.0.4)

Builds pybind11's upstream CMake extension test, then imports the module in Python and calls its bound C++ function.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.37 | 0.37 | 🟡 1.01x [0.99x, 1.02x] | |
| compile | 2.63 | 1.13 | 🟢 2.34x [2.34x, 2.35x] | |
| run tests | 0.01 | 0.01 | 🟢 1.09x [1.07x, 1.13x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.37 | 0.37 | 🟡 1.00x [0.99x, 1.01x] | |
| compile | 3.04 | 1.86 | 🟢 1.63x [1.63x, 1.64x] | |
| run tests | 0.01 | 0.01 | 🟢 1.08x [1.05x, 1.10x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
