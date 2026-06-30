# Cppcheck Compile-Time Comparison

psychicstd vs system libstdc++ (GCC 15)

| Source file | psychicstd (ms) | libstdc++ (ms) | Speedup |
|-------------|-----------------|----------------|---------|
| addoninfo | — | — | — |
| analyzerinfo | 188 | 439 | 2.3x |
| color | 80 | 168 | 2.1x |
| timer | 163 | 764 | 4.7x |
| errortypes | 58 | 242 | 4.2x |
| astutils | 661 | 1354 | 2.0x |
| checkassert | 166 | 558 | 3.4x |
| checkbool | 196 | 556 | 2.8x |
| checkcondition | 437 | 943 | 2.2x |
| checkfunctions | 274 | 679 | 2.5x |
| tokenize | 1494 | 3154 | 2.1x |
| symboldatabase | 1339 | 2653 | 2.0x |
| valueflow | 1844 | 3889 | 2.1x |
| checkclass | 792 | 1560 | 2.0x |
| checkbufferoverrun | 375 | 908 | 2.4x |
| cmdlineparser | — | — | — |
| filelister | 119 | 403 | 3.4x |
| cppcheckexecutor | 321 | 1114 | 3.5x |
| **Total (16 files)** | **8507** | **19384** | **2.3x** |

— = excluded (requires CMake-generated config.h)

16 of 18 selected source files compile standalone. Total time: 8.5s psychicstd vs 19.4s libstdc++ — a 2.3x speedup.

Generated 2026-07-01T00:10:51+02:00
