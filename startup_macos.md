# Process Startup Speed

Median of 300 runs (after 20 warmup runs) of a trivial program (`benchmarks/startup_time/bench_startup.cpp`) linked against the system STL vs psychicstd. This measures exec-to-exit wall time, i.e. dynamic linker + startup overhead -- separate from compile time, and separate from the program's own work.

Last updated: 2026-07-15 19:42

Platform: macOS 26 arm64 (Apple M3), Apple clang 21, system STL = libc++.

| | median startup | shared libraries |
|--|---:|---|
| system | 1.934 ms | libSystem.B.dylib, libc++.1.dylib |
| psychicstd | 1.656 ms | libSystem.B.dylib, libc++abi.dylib |

Speedup: **1.17x**
