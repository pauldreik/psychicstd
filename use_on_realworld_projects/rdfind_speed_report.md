# rdfind compile speed test

Median of 3 runs. Both builds use `-std=c++20`.
Speedup = system time / psychicstd time (>1x means psychicstd is faster).
make check measures runtime, not compile time.

## Debug (-O0)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.24 | 1.10 | 1.13x |
| make | 3.41 | 0.99 | 3.45x |
| make check | 2.50 | 14.00 | 0.18x |

## Release (-O2)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.25 | 1.11 | 1.13x |
| make | 4.14 | 2.09 | 1.98x |
| make check | 2.37 | 1.71 | 1.39x |
