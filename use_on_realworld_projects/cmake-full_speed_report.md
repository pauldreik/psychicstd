# Real-world project speed comparison

Compiler: `g++-14 (Debian 14.2.0-19) 14.2.0`. Each project is built 1 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## cmake-full (4.3.4 (full))

Builds the cmake, ctest, and cpack executables and runs the self-contained CMakeLib, CMakeOnly, command-language, and core RunCMake tests. OpenSSL and debugger support are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 21.58 | 19.45 | 🟡 1.11x | |
| compile | 114.77 | 38.48 | 🟡 2.98x | |
| run tests | 15.30 | 13.84 | 🟡 1.11x | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler g++-14 --build-type debug --reps 1 --jobs 8`
