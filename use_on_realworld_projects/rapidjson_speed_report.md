# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 6 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 22 jobs at 1.5 GiB/job). ccache was disabled.

## rapidjson (master-24b5e7a8b27f)

RapidJSON's examples, archivertest, and unit tests are built; simpledom and unittest are run.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.45 | 0.45 | 🟡 0.99x [0.98x, 1.02x] | |
| compile | 11.70 | 9.43 | 🟢 1.24x [1.23x, 1.26x] | |
| run example | 0.00 | 0.00 | 🟢 1.58x [1.36x, 1.73x] | |
| run tests | 2.22 | 2.24 | 🔴 0.99x [0.97x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.45 | 0.45 | 🟡 1.00x [0.99x, 1.02x] | |
| compile | 46.60 | 48.04 | 🔴 0.97x [0.96x, 0.97x] | |
| run example | 0.00 | 0.00 | 🟢 1.84x [1.44x, 2.02x] | |
| run tests | 0.33 | 0.33 | 🟡 1.00x [0.99x, 1.01x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 6 --jobs 8`
