# Real-world project speed comparison

Compiler: `g++-14 (Debian 14.2.0-19) 14.2.0`. Each project is built 1 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## pybind11-full (3.0.4 (full))

Builds pybind11's complete primary C++/pytest module set and runs its upstream pytest suite; optional Eigen, Boost, Catch, and SciPy integrations are excluded.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 1.53 | 0.77 | 🟡 1.97x | |
| compile | 64.98 | 36.15 | 🟡 1.80x | |
| run tests | 23.23 | 17.35 | 🟡 1.34x | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler g++-14 --build-type debug --reps 1 --jobs 8`
