# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 3 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## cmake (4.3.4)

Builds upstream CMake's core static library together with its KWSys, std-compatibility, and JSON support targets, then runs the supported KWSys tests. OpenSSL and debugger support are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 21.29 | 18.85 | 🟢 1.13x [1.09x, 1.17x] | |
| compile | 67.04 | 20.82 | 🟢 3.22x [3.12x, 3.22x] | |
| run tests | 1.17 | 1.17 | 🟢 1.00x [1.00x, 1.01x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 21.14 | 18.30 | 🟢 1.16x [1.13x, 1.22x] | |
| compile | 65.96 | 33.19 | 🟢 1.99x [1.93x, 2.01x] | |
| run tests | 1.17 | 1.17 | 🟡 1.00x [1.00x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 3`
