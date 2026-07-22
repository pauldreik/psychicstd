# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 1 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (20 logical CPUs available; the memory estimate permits 21 jobs at 1.5 GiB/job). ccache was disabled.

## opencv (4.13.0)

Builds OpenCV's core and imgproc modules and runs their upstream tests; optional codecs, bindings, and hardware backends are disabled.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 17.29 | 17.19 | 🟡 1.01x | |
| compile | 67.26 | 38.36 | 🟡 1.75x | |
| run tests | 493.43 | 491.04 | 🟡 1.00x | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| configure | 17.60 | 17.55 | 🟡 1.00x | |
| compile | 114.60 | 92.34 | 🟡 1.24x | |
| run tests | 117.81 | 119.02 | 🟡 0.99x | |

______________________________________________________________________

Reproduce this on your machine: `scripts/compare_realworld_performance.py --compiler c++ --build-type both --reps 1 --jobs 8`
