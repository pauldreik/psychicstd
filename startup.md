# Process Startup Speed

Median of 300 runs (after 20 warmup runs) of a trivial program (`benchmarks/startup_time/bench_startup.cpp`) linked against the system STL vs psychicstd. This measures exec-to-exit wall time, i.e. dynamic linker + startup overhead -- separate from compile time, and separate from the program's own work.

Last updated: 2026-07-12 21:57

| | median startup | shared libraries |
|--|---:|---|
| system | 0.919 ms | libc.so.6, libgcc_s.so.1, libm.so.6, libstdc++.so.6 |
| psychicstd | 0.510 ms | libc.so.6, libgcc_s.so.1 |

Speedup: **1.80x**
