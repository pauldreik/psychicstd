# Eigen Test Compile-Time Comparison

psychicstd vs system libstdc++ (GCC 15)

| Test | psychicstd (ms) | libstdc++ (ms) | Speedup |
|------|-----------------|----------------|---------|
| basicstuff | 5869 | 6840 | 1.2x |
| meta | 412 | 1012 | 2.5x |
| numext | 539 | 1113 | 2.1x |
| block | 342 | 929 | 2.7x |
| corners | 336 | 932 | 2.8x |
| determinant | 335 | 917 | 2.7x |
| diagonal | 325 | 908 | 2.8x |
| array_cwise | 363 | 935 | 2.6x |
| array_for_matrix | 349 | 932 | 2.7x |
| constructor | 480 | 1084 | 2.3x |
| adjoint | 421 | 1006 | 2.4x |
| triangular | 367 | 950 | 2.6x |
| **Average (12 tests)** | **844** | **1463** | **1.7x** |

⚠ = compiled but test run failed (not included in average)

Generated 2026-06-30T16:35:55+02:00
