# Process Startup Speed

Median of 3 batches of 3000 paired runs (after 20 warmup pairs per batch) of a representative small program (`benchmarks/startup_time/bench_startup.cpp`) linked against the system STL vs psychicstd. This measures exec-to-exit wall time, including dynamic loading, runtime initialization, and the program's fixed workload. It is not an isolated measurement of dynamic-linker time.

psychicstd is linked as a static archive: required archive members are copied into the executable, so `libpsychicstd.a` is not a startup-time shared-library dependency. The table lists shared libraries reported by the platform dependency tool.

Last updated: 2026-07-22 20:11

| | median exec-to-exit | shared libraries |
|--|---:|---|
| system | 0.943 ms | libc.so.6, libgcc_s.so.1, libm.so.6, libstdc++.so.6 |
| psychicstd | 0.555 ms | libc.so.6, libgcc_s.so.1 |

Speedup: **1.70x**
