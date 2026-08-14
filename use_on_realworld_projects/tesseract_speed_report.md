# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 6 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 17 jobs at 1.5 GiB/job). ccache was disabled.

## tesseract (5.5.2)

Builds the Tesseract OCR library and command-line program, then runs its version check. Training tools, ScrollView graphics, libarchive, libcurl, and OpenMP are disabled; Leptonica is required.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 17.11 | 8.19 | 🟢 2.09x [2.06x, 2.13x] | |
| run tests | 0.00 | 0.00 | 🔴 0.94x [0.87x, 1.00x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 18.08 | 11.74 | 🟢 1.54x [1.52x, 1.55x] | |
| run tests | 0.00 | 0.00 | 🔴 0.92x [0.86x, 0.97x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 6 --jobs 8`
