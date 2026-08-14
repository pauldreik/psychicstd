# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 16 jobs at 1.5 GiB/job). ccache was disabled.

## boost-asio (1.91.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 25.03 | 25.04 | 🟡 1.00x [1.00x, 1.01x] | |
| compile | 3.22 | 1.55 | 🟢 2.08x [2.06x, 2.09x] | Representative upstream Asio tests are compiled and linked directly; unrelated Boost libraries are excluded. |
| run tests | 8.01 | 8.01 | 🟢 1.00x [1.00x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 25.01 | 25.00 | 🟡 1.00x [1.00x, 1.01x] | |
| compile | 3.22 | 1.54 | 🟢 2.09x [2.08x, 2.10x] | Representative upstream Asio tests are compiled and linked directly; unrelated Boost libraries are excluded. |
| run tests | 8.01 | 8.01 | 🟢 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
