# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 9 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 16 jobs at 1.5 GiB/job). ccache was disabled.

## electron (43.2.0)

Builds Electron's core command-line validation and original argument storage unchanged, then runs focused checks for the protocol-handler argument guard, sandbox switches, and Linux command line initialization. Small Chromium interface shims keep this initial slice independent of the full Chromium checkout.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 0.85 | 0.21 | 🟢 4.08x [3.98x, 4.13x] | |
| run tests | 0.00 | 0.00 | 🟢 1.75x [1.70x, 1.84x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 0.98 | 0.32 | 🟢 3.02x [3.00x, 3.05x] | |
| run tests | 0.00 | 0.00 | 🟢 1.67x [1.50x, 1.82x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 9 --jobs 8`
