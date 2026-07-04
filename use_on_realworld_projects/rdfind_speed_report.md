# rdfind compile speed test

Median of 3 runs. Both builds use `-std=c++20`.
Speedup = system time / psychicstd time (>1x means psychicstd is faster).
make check measures runtime, not compile time.

## Debug (-O0)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.11 | 0.98 | 1.13x |
| make | 2.61 | 0.92 | 2.85x |
| make check | 2.27 | 6.18 | 0.37x |

## Release (-O2)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.10 | 0.98 | 1.12x |
| make | 3.36 | 1.62 | 2.07x |
| make check | 2.17 | 1.26 | 1.72x |
