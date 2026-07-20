# Real-world project speed comparison

Compiler: `c++ (Ubuntu 12.3.0-1ubuntu1~22.04.3) 12.3.0`. Each project is built 5 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## rapidjson (master-24b5e7a8b27f)

RapidJSON's examples and archivertest are built; simpledom is run. The upstream unit tests currently require std::string allocator construction not yet supported by psychicstd.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.59 | 0.58 | 🟡 1.02x [0.93x, 1.06x] | |
| compile | 6.64 | 3.74 | 🟢 1.78x [1.47x, 2.15x] | |
| run example | 0.00 | 0.00 | 🟢 2.08x [1.30x, 2.85x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 0.58 | 0.58 | 🟡 1.00x [0.98x, 1.06x] | |
| compile | 11.21 | 9.62 | 🟢 1.16x [1.15x, 1.18x] | |
| run example | 0.00 | 0.00 | 🟢 1.75x [1.16x, 2.31x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 5`
