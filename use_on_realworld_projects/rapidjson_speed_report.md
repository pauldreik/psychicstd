# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## rapidjson (master-24b5e7a8b27f)

RapidJSON's examples, archivertest, and unit tests are built; simpledom and unittest are run.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.46 | 0.47 | 🟡 0.98x [0.97x, 1.01x] | |
| compile | 11.43 | 9.69 | 🟢 1.18x [1.17x, 1.20x] | |
| run example | 0.00 | 0.00 | 🟢 1.63x [1.27x, 1.80x] | |
| run tests | 2.24 | 2.29 | 🔴 0.98x [0.97x, 0.99x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.46 | 0.46 | 🟡 1.01x [0.98x, 1.02x] | |
| compile | 45.30 | 46.64 | 🔴 0.97x [0.97x, 0.98x] | |
| run example | 0.00 | 0.00 | 🟢 1.51x [1.30x, 1.65x] | |
| run tests | 0.34 | 0.33 | 🟢 1.02x [1.01x, 1.03x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 5 --jobs 8`
