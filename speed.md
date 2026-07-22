# Compilation Speed

Median of 3 compilations per file. Ordered by system STL compile time, slowest first.

Compiler: `g++-14 (Debian 14.2.0-19) 14.2.0`

The interval is a bootstrapped 95% confidence interval for the system/psychicstd speedup ratio.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-07-23 01:22

| | name | system | psychicstd | speedup | 95% CI |
|--|------|-------:|----------:|--------:|-------:|
| 🟢 | regex | 1331.4ms | 52.6ms | 25.31x | [24.71x, 25.60x] |
| 🟢 | all-headers | 1133.5ms | 125.2ms | 9.06x | [8.89x, 9.65x] |
| 🟢 | memory | 746.2ms | 128.8ms | 5.79x | [5.74x, 5.83x] |
| 🟢 | future | 723.0ms | 66.2ms | 10.92x | [10.26x, 11.10x] |
| 🟢 | filesystem | 641.2ms | 104.4ms | 6.14x | [5.98x, 6.19x] |
| 🟢 | chrono | 602.5ms | 45.7ms | 13.19x | [12.90x, 13.50x] |
| 🟢 | wordcounter | 599.5ms | 107.8ms | 5.56x | [5.46x, 5.68x] |
| 🟢 | thread | 592.0ms | 117.1ms | 5.06x | [4.97x, 5.08x] |
| 🟢 | mutex | 558.8ms | 74.9ms | 7.46x | [7.37x, 7.60x] |
| 🟢 | atomic | 534.4ms | 68.7ms | 7.77x | [7.26x, 7.88x] |
| 🟢 | stop_token | 492.1ms | 45.7ms | 10.77x | [10.63x, 10.93x] |
| 🟢 | algorithm | 483.4ms | 143.6ms | 3.37x | [3.35x, 3.47x] |
| 🟢 | complex | 481.4ms | 35.4ms | 13.59x | [13.31x, 13.73x] |
| 🟢 | iostream | 452.9ms | 110.6ms | 4.10x | [4.07x, 4.12x] |
| 🟢 | iomanip | 446.7ms | 73.6ms | 6.07x | [5.98x, 6.17x] |
| 🟢 | istream | 444.5ms | 77.2ms | 5.76x | [5.57x, 5.83x] |
| 🟢 | locale | 438.3ms | 82.9ms | 5.29x | [5.22x, 5.33x] |
| 🟢 | fstream | 435.5ms | 86.4ms | 5.04x | [4.92x, 5.13x] |
| 🟢 | sstream | 435.2ms | 68.6ms | 6.34x | [6.31x, 6.42x] |
| 🟢 | bench/iostream | 402.1ms | 46.5ms | 8.65x | [8.62x, 9.03x] |
| 🟢 | variant | 312.2ms | 75.3ms | 4.14x | [4.13x, 4.17x] |
| 🟢 | random | 309.7ms | 66.9ms | 4.63x | [4.63x, 4.67x] |
| 🟢 | ranges | 272.7ms | 34.0ms | 8.02x | [7.76x, 8.04x] |
| 🟢 | map | 259.8ms | 110.1ms | 2.36x | [2.34x, 2.37x] |
| 🟢 | string | 258.7ms | 57.5ms | 4.50x | [4.47x, 4.56x] |
| 🟢 | vector | 227.4ms | 67.4ms | 3.37x | [3.30x, 3.39x] |
| 🟢 | queue | 205.3ms | 71.9ms | 2.85x | [2.84x, 2.88x] |
| 🟢 | iterator | 199.2ms | 34.2ms | 5.82x | [5.64x, 6.03x] |
| 🟢 | valarray | 188.5ms | 16.6ms | 11.38x | [11.37x, 11.76x] |
| 🟢 | system_error | 187.6ms | 46.9ms | 4.00x | [3.99x, 4.08x] |
| 🟢 | set | 183.6ms | 70.4ms | 2.61x | [2.51x, 2.65x] |
| 🟢 | ios | 178.5ms | 45.8ms | 3.90x | [3.83x, 3.95x] |
| 🟢 | unordered_map | 177.0ms | 49.4ms | 3.58x | [3.53x, 3.62x] |
| 🟢 | deque | 172.7ms | 52.2ms | 3.31x | [3.28x, 3.34x] |
| 🟢 | string_view | 171.6ms | 54.7ms | 3.14x | [3.06x, 3.16x] |
| 🟢 | span | 171.2ms | 41.7ms | 4.10x | [4.02x, 4.14x] |
| 🟢 | typeinfo | 164.2ms | 39.2ms | 4.19x | [4.14x, 4.23x] |
| 🟢 | stdexcept | 161.4ms | 39.0ms | 4.13x | [4.04x, 4.17x] |
| 🟢 | functional | 159.5ms | 28.3ms | 5.63x | [5.50x, 5.80x] |
| 🟢 | bitset | 154.4ms | 23.7ms | 6.53x | [6.47x, 6.85x] |
| 🟢 | tuple | 146.3ms | 49.5ms | 2.95x | [2.92x, 2.99x] |
| 🟢 | unordered_set | 145.2ms | 31.4ms | 4.63x | [4.53x, 4.66x] |
| 🟢 | list | 132.3ms | 39.7ms | 3.33x | [3.30x, 3.39x] |
| 🟢 | stack | 124.8ms | 32.9ms | 3.79x | [3.77x, 3.86x] |
| 🟢 | cmath | 104.4ms | 35.0ms | 2.98x | [2.84x, 3.19x] |
| 🟢 | forward_list | 95.6ms | 22.7ms | 4.22x | [4.14x, 4.40x] |
| 🟢 | optional | 75.6ms | 35.4ms | 2.14x | [2.05x, 2.15x] |
| 🟢 | array | 51.3ms | 18.9ms | 2.71x | [2.48x, 2.77x] |
| 🟢 | any | 50.1ms | 31.0ms | 1.61x | [1.55x, 1.64x] |
| 🟢 | numeric | 41.5ms | 16.1ms | 2.58x | [2.55x, 2.66x] |
| 🟢 | utility | 34.3ms | 16.5ms | 2.07x | [1.99x, 2.12x] |
| 🟢 | compare | 29.3ms | 15.4ms | 1.91x | [1.80x, 1.97x] |
| 🟢 | exception | 28.8ms | 16.2ms | 1.78x | [1.77x, 1.94x] |
| 🟢 | ratio | 23.1ms | 14.8ms | 1.56x | [1.52x, 1.61x] |
| 🟢 | concepts | 20.5ms | 16.7ms | 1.23x | [1.19x, 1.36x] |
| 🟡 | cstdlib | 20.4ms | 18.1ms | 1.13x | [1.06x, 1.18x] |
| 🟢 | type_traits | 19.8ms | 14.3ms | 1.38x | [1.32x, 1.55x] |
| 🟢 | cstdlib_order | 19.6ms | 15.7ms | 1.25x | [1.13x, 1.28x] |
| 🟢 | limits | 18.2ms | 13.9ms | 1.31x | [1.27x, 1.42x] |
| 🟡 | new | 13.9ms | 12.2ms | 1.14x | [1.08x, 1.21x] |
| 🟡 | initializer_list | 13.8ms | 12.5ms | 1.11x | [0.98x, 1.22x] |
| 🟡 | cstddef | 13.7ms | 14.4ms | 0.95x | [0.86x, 1.07x] |
| 🟡 | uchar | 12.9ms | 12.2ms | 1.06x | [1.01x, 1.19x] |
| 🟡 | cstdint | 12.6ms | 11.1ms | 1.13x | [1.12x, 1.31x] |
