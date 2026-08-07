# Process Startup Speed

Median of 3 batches of 300 paired runs (after 20 warmup pairs per batch) of a representative small program (`benchmarks/startup_time/bench_startup.cpp`) linked against the system STL vs psychicstd. This measures exec-to-exit wall time, including dynamic loading, runtime initialization, and the program's fixed workload. It is not an isolated measurement of dynamic-linker time.

psychicstd is linked as a static archive: required archive members are copied into the executable, so `libpsychicstd.a` is not a startup-time shared-library dependency. The table lists shared libraries reported by the platform dependency tool.

The interval is a paired bootstrap 95% confidence interval. Each bootstrap sample resamples the paired runs within each batch.

Last updated: 2026-08-07 03:06

| | median exec-to-exit | shared libraries |
|--|---:|---|
| system | 0.892 ms | libc.so.6, libgcc_s.so.1, libm.so.6, libstdc++.so.6 |
| psychicstd | 0.528 ms | libc.so.6, libgcc_s.so.1 |

Speedup: **1.69x** (95% CI: **[1.68x, 1.71x]**)

______________________________________________________________________

Reproduce on your machine:

`cmake -B build/ -S . -DCMAKE_BUILD_TYPE=Debug`

`cmake --build build/ --target startup_bench`
