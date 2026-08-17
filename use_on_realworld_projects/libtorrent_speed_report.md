# Real-world project speed comparison

Compiler: `c++ (Debian 14.2.0-19) 14.2.0`. Each project is built 4 time(s) per side (system libstdc++, psychicstd); `system (s)`/`psychicstd (s)` are the *median* build time of those repetitions, in seconds -- the median is used instead of the mean so one repetition disturbed by another process on the machine doesn't skew the result. `speedup` = system median / psychicstd median (>1x means psychicstd is faster); its bracketed range is a 95% confidence interval on that *same ratio* (obtained by resampling the raw per-repetition timings, not just the two medians, 2000 times) -- so it reflects how much the repetitions varied, not a different unit. 🟢 the whole CI is above 1x (reliably faster) · 🔴 the whole CI is below 1x (reliably slower) · 🟡 the CI straddles 1x (not distinguishable from run-to-run noise).

Parallelism: **8 jobs** (16 logical CPUs available; the memory estimate permits 15 jobs at 1.5 GiB/job). ccache was disabled.

## libtorrent (2.1.0)

Builds the complete static libtorrent library with DHT, encryption, extensions, I2P, logging, and streaming enabled. WebTorrent is disabled because its bundled submodule dependencies are not part of the release archive. Enables libtorrent's movable deque-element fallback because psychicstd's compact deque relocates elements when it grows.

### Debug

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 53.07 | 27.67 | 🟢 1.92x [1.90x, 1.93x] | |

### Release

| step | system (s) | psychicstd (s) | speedup | comment |
| --- | ---: | ---: | ---: | --- |
| compile | 53.68 | 31.79 | 🟢 1.69x [1.68x, 1.70x] | |

______________________________________________________________________

Reproduce this on your machine: `scripts/benchmark_realworld.py --compiler c++ --build-type both --reps 4 --jobs 8`
