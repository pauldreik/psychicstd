# Cppcheck Compile-Time Comparison

psychicstd vs system libstdc++ (GCC 14)

| Source file | psychicstd (ms) | libstdc++ (ms) | Speedup |
|-------------|-----------------|----------------|---------|
| addoninfo | — | — | — |
| analyzerinfo | 218 | 546 | 2.5x |
| color | 99 | 206 | 2.1x |
| timer | 181 | 960 | 5.3x |
| errortypes | 68 | 296 | 4.4x |
| astutils | 729 | 1586 | 2.2x |
| checkassert | 191 | 690 | 3.6x |
| checkbool | 202 | 706 | 3.5x |
| checkcondition | 487 | 1135 | 2.3x |
| checkfunctions | 294 | 863 | 2.9x |
| tokenize | 1582 | 3694 | 2.3x |
| symboldatabase | 1424 | 3055 | 2.1x |
| valueflow | 1861 | 4515 | 2.4x |
| checkclass | 845 | 1813 | 2.1x |
| checkbufferoverrun | 424 | 1126 | 2.7x |
| cmdlineparser | — | — | — |
| filelister | 137 | 498 | 3.6x |
| cppcheckexecutor | 365 | 1449 | 4.0x |
| **Total (16 files)** | **9107** | **23138** | **2.5x** |

Generated 2026-07-07T07:39:39+02:00
