# Eigen Test Compile-Time Comparison

psychicstd vs system libstdc++ (GCC 14)

| Test | psychicstd (ms) | libstdc++ (ms) | Speedup |
|------|-----------------|----------------|---------|
| basicstuff | 6697 | 7892 | 1.2x |
| meta | 494 | 1288 | 2.6x |
| numext | 608 | 1416 | 2.3x |
| block | 390 | 1232 | 3.2x |
| corners | 389 | 1225 | 3.1x |
| determinant | 390 | 1215 | 3.1x |
| diagonal | 392 | 1242 | 3.2x |
| array_cwise | 423 | 1281 | 3.0x |
| array_for_matrix | 436 | 1258 | 2.9x |
| constructor | 587 | 1426 | 2.4x |
| adjoint | 502 | 1354 | 2.7x |
| triangular | 431 | 1270 | 2.9x |
| **Average (12 tests)** | **978** | **1841** | **1.9x** |

⚠ = compiled but test run failed (not included in average)

Generated 2026-07-07T07:40:20+02:00
