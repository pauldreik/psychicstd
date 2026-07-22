# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (8 logical CPUs available; the memory estimate permits 22 jobs at 1.5 GiB/job). ccache was disabled.

## rapidjson (master-24b5e7a8b27f)

RapidJSON's examples, archivertest, and unit tests are built; simpledom and unittest are run.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.43 | 0.43 | 🟡 1.00x [0.99x, 1.01x] | |
| compile | 11.39 | 9.34 | 🟢 1.22x [1.22x, 1.22x] | |
| run example | 0.00 | 0.00 | 🟢 1.47x [1.33x, 1.54x] | |
| run tests | 2.22 | 2.29 | 🔴 0.97x [0.97x, 0.99x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.43 | 0.43 | 🟡 1.00x [0.99x, 1.01x] | |
| compile | 44.40 | 45.84 | 🔴 0.97x [0.96x, 0.97x] | |
| run example | 0.00 | 0.00 | 🟢 1.35x [1.30x, 1.73x] | |
| run tests | 0.34 | 0.33 | 🟢 1.03x [1.02x, 1.04x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3`
