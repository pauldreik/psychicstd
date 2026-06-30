# Catch2 Compile-Time Comparison

psychicstd vs system libstdc++ (GCC 15)

| Metric | psychicstd | libstdc++ | Speedup |
|--------|-----------|-----------|---------|
| Build time | 9761ms | 15547ms | 1.6x |
| Tests passed | 69 | — | — |
| Tests failed | 69 | — | — |

ApprovalTests excluded (stdout/stderr ordering differs with psychicstd's buffered cerr/clog).

Generated 2026-07-01T00:38:37+02:00
