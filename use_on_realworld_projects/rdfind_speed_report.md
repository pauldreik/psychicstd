# rdfind compile speed test

Median of 3 runs. Both builds use `-std=c++20`.
Speedup = system time / psychicstd time (>1x means psychicstd is faster).
make check measures runtime, not compile time.

## Debug (-O0)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.27 | 1.12 | 1.14x |
| make | 3.23 | 1.08 | 2.98x |
| make check | 2.48 | 6.10 | 0.41x |

## Release (-O2)

| step | system (s) | psychicstd (s) | speedup |
| --- | ---: | ---: | ---: |
| configure | 1.29 | 1.16 | 1.11x |
| make | 4.01 | 1.99 | 2.01x |
| make check | 2.59 | 1.49 | 1.74x |
