# Real-world project speed comparison

Compiler: `Apple clang version 21.0.0 (clang-2100.0.123.102)`. Each project is built 7 time(s) per side (system libc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

## rdfind (commit 787b01ab378c)

rdfind is an autoconf based project. It uses psychic strict mode.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 2.53 | 2.40 | 🟢 1.05x [1.03x, 1.09x] | |
| compile | 0.88 | 0.19 | 🟢 4.72x [4.14x, 5.07x] | |
| run tests | 11.68 | 5.13 | 🟢 2.28x [2.21x, 2.32x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 2.42 | 2.35 | 🟡 1.03x [0.99x, 1.08x] | |
| compile | 0.96 | 0.36 | 🟢 2.68x [2.52x, 2.94x] | |
| run tests | 4.84 | 4.93 | 🔴 0.98x [0.95x, 1.00x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler /usr/bin/clang++ --build-type both --reps 7`
