# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 12 jobs at 1.5 GiB/job). ccache was disabled.

## date (3.0.4)

Builds date's time-zone library and portable test suite using date's C-locale mode; tests importing std::chrono wholesale are excluded because their unqualified backport names conflict in C++20, as is one custom clock_cast test affected by C++20 overload resolution.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 7.05 | 3.07 | 🟢 2.30x [2.23x, 2.40x] | |
| run tests | 5.32 | 5.79 | 🔴 0.92x [0.85x, 0.99x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 7.48 | 4.86 | 🟢 1.54x [1.47x, 1.63x] | |
| run tests | 0.10 | 0.10 | 🟡 1.02x [0.90x, 1.09x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 3 --jobs 8`
