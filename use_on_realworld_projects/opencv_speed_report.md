# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 4 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 17 jobs at 1.5 GiB/job). ccache was disabled.

## opencv (4.13.0)

Builds OpenCV's core and imgproc modules, including their test executables, without running the tests. Optional codecs, bindings, and hardware backends are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 18.48 | 18.31 | 🟢 1.01x [1.00x, 1.02x] | |
| compile | 67.50 | 37.52 | 🟢 1.80x [1.79x, 1.81x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 18.76 | 18.90 | 🟡 0.99x [0.98x, 1.01x] | |
| compile | 114.92 | 93.35 | 🟢 1.23x [1.23x, 1.24x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 4 --jobs 8`
