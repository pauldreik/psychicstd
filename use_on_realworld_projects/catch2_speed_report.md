# Catch2 Compile-Time Comparison

psychicstd vs system libstdc++ (GCC 14)

| Metric | psychicstd | libstdc++ | Speedup |
|--------|-----------|-----------|---------|
| Build time | 8747ms | 15736ms | 1.8x |
| Tests passed | 70 | — | — |
| Tests failed | 70 | — | — |

ApprovalTests excluded (stdout/stderr ordering differs with psychicstd's buffered cerr/clog).

Generated 2026-07-07T07:38:51+02:00
