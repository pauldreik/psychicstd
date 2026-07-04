# rdfind compile speed test

Median of 3 runs. Both builds use `-std=c++20`.
Speedup = system time / psychicstd time (>1x means psychicstd is faster).
make check measures runtime, not compile time.

## Debug (-O0)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.05 | 0.91 | 1.16x |
| make | 2.56 | 0.92 | 2.77x |
| make check | 2.27 | 13.21 | 0.17x |

## Release (-O2)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.04 | 0.92 | 1.14x |
| make | 3.31 | 1.65 | 2.01x |
| make check | 2.20 | 1.23 | 1.79x |
