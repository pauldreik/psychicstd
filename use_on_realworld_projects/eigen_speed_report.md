# Eigen Test Compile-Time Comparison

psychicstd vs system libstdc++ (GCC 15)

| Test | psychicstd (ms) | libstdc++ (ms) | Speedup |
|------|-----------------|----------------|---------|
| basicstuff | 5922 | 6940 | 1.2x |
| meta | 415 | 996 | 2.4x |
| numext | 538 | 1118 | 2.1x |
| block | 342 | 919 | 2.7x |
| corners | 333 | 917 | 2.8x |
| determinant | 342 | 917 | 2.7x |
| diagonal | 332 | 930 | 2.8x |
| array_cwise | 354 | 940 | 2.7x |
| array_for_matrix | 367 | 940 | 2.6x |
| constructor | 495 | 1074 | 2.2x |
| adjoint | 426 | 1009 | 2.4x |
| triangular | 378 | 955 | 2.5x |
| **Average (12 tests)** | **853** | **1471** | **1.7x** |

⚠ = compiled but test run failed (not included in average)

Generated 2026-07-01T00:36:53+02:00
