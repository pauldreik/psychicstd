# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **20 jobs** (20 logical CPUs available; the memory estimate permits 22 jobs at 1.5 GiB/job). ccache was disabled.

## boost-asio (1.91.0)

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 24.68 | 24.65 | 🟡 1.00x [0.97x, 1.02x] | |
| compile | 3.16 | 1.62 | 🟢 1.96x [1.94x, 1.97x] | Representative upstream Asio tests are compiled and linked directly; unrelated Boost libraries are excluded. |
| run tests | 8.01 | 8.01 | 🟢 1.00x [1.00x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 24.65 | 24.68 | 🟡 1.00x [0.98x, 1.03x] | |
| compile | 3.19 | 1.62 | 🟢 1.97x [1.93x, 2.06x] | Representative upstream Asio tests are compiled and linked directly; unrelated Boost libraries are excluded. |
| run tests | 8.01 | 8.01 | 🟢 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5`
