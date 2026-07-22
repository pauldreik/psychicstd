# Compilation Speed

Median of 10 compilations per file. Ordered by system STL compile time, slowest first.

🟢 above 1.2x 🟡 0.8x–1.2x 🔴 below 0.8x

Last updated: 2026-07-22 10:36

| | name | system | psychicstd | speedup |
|--|------|-------:|----------:|--------:|
| 🟢 | regex | 1325.5ms | 62.6ms | 21.2x |
| 🟢 | memory | 757.0ms | 132.7ms | 5.7x |
| 🟢 | future | 744.1ms | 69.7ms | 10.7x |
| 🟢 | filesystem | 685.7ms | 148.0ms | 4.6x |
| 🟢 | chrono | 602.3ms | 47.2ms | 12.8x |
| 🟢 | wordcounter | 602.1ms | 149.6ms | 4.0x |
| 🟢 | thread | 590.8ms | 156.2ms | 3.8x |
| 🟢 | mutex | 573.3ms | 73.8ms | 7.8x |
| 🟢 | atomic | 542.8ms | 67.6ms | 8.0x |
| 🟢 | complex | 497.8ms | 35.9ms | 13.9x |
| 🟢 | iostream | 493.5ms | 163.0ms | 3.0x |
| 🟢 | stop_token | 487.6ms | 45.3ms | 10.8x |
| 🟢 | algorithm | 481.0ms | 141.2ms | 3.4x |
| 🟢 | iomanip | 449.2ms | 118.9ms | 3.8x |
| 🟢 | istream | 445.6ms | 123.7ms | 3.6x |
| 🟢 | fstream | 443.3ms | 118.5ms | 3.7x |
| 🟢 | locale | 438.1ms | 101.9ms | 4.3x |
| 🟢 | sstream | 435.5ms | 118.6ms | 3.7x |
| 🟢 | random | 312.4ms | 69.8ms | 4.5x |
| 🟢 | variant | 310.7ms | 84.1ms | 3.7x |
| 🟢 | ranges | 273.1ms | 34.4ms | 7.9x |
| 🟢 | map | 266.4ms | 117.3ms | 2.3x |
| 🟢 | string | 230.8ms | 79.3ms | 2.9x |
| 🟢 | vector | 226.6ms | 67.7ms | 3.3x |
| 🟢 | queue | 203.2ms | 72.1ms | 2.8x |
| 🟢 | iterator | 200.3ms | 34.8ms | 5.7x |
| 🟢 | set | 190.8ms | 72.3ms | 2.6x |
| 🟢 | valarray | 187.2ms | 16.3ms | 11.5x |
| 🟢 | system_error | 185.6ms | 67.2ms | 2.8x |
| 🟢 | deque | 183.4ms | 59.0ms | 3.1x |
| 🟢 | unordered_map | 177.6ms | 49.7ms | 3.6x |
| 🟢 | ios | 174.8ms | 67.4ms | 2.6x |
| 🟢 | string_view | 170.2ms | 61.0ms | 2.8x |
| 🟢 | span | 168.4ms | 42.1ms | 4.0x |
| 🟢 | typeinfo | 163.5ms | 52.1ms | 3.1x |
| 🟢 | functional | 160.2ms | 29.3ms | 5.5x |
| 🟢 | stdexcept | 156.6ms | 42.7ms | 3.7x |
| 🟢 | bitset | 155.2ms | 23.5ms | 6.6x |
| 🟢 | tuple | 146.9ms | 49.7ms | 3.0x |
| 🟢 | unordered_set | 145.9ms | 31.6ms | 4.6x |
| 🟢 | list | 134.9ms | 42.9ms | 3.1x |
| 🟢 | stack | 123.8ms | 33.6ms | 3.7x |
| 🟢 | cmath | 108.5ms | 35.4ms | 3.1x |
| 🟢 | forward_list | 98.2ms | 22.6ms | 4.4x |
| 🟢 | optional | 74.4ms | 36.0ms | 2.1x |
| 🟢 | array | 49.3ms | 18.5ms | 2.7x |
| 🟢 | all-headers | 46.8ms | 17.0ms | 2.7x |
| 🟢 | any | 45.2ms | 30.9ms | 1.5x |
| 🟢 | numeric | 42.8ms | 16.6ms | 2.6x |
| 🟢 | bench/numeric | 41.4ms | 14.9ms | 2.8x |
| 🟢 | exception | 33.7ms | 18.7ms | 1.8x |
| 🟢 | utility | 33.4ms | 16.5ms | 2.0x |
| 🟢 | compare | 29.6ms | 16.6ms | 1.8x |
| 🟢 | ratio | 22.0ms | 15.0ms | 1.5x |
| 🟢 | concepts | 21.3ms | 17.6ms | 1.2x |
| 🟡 | cstdlib | 20.3ms | 19.1ms | 1.1x |
| 🟢 | type_traits | 20.3ms | 13.5ms | 1.5x |
| 🟡 | limits | 19.2ms | 16.0ms | 1.2x |
| 🟡 | cstdlib_order | 17.5ms | 16.3ms | 1.1x |
| 🟡 | new | 15.3ms | 12.9ms | 1.2x |
| 🟡 | cstddef | 14.9ms | 15.0ms | 1.0x |
| 🟡 | initializer_list | 14.1ms | 13.2ms | 1.1x |
| 🟡 | cstdint | 13.3ms | 11.6ms | 1.1x |
| 🟡 | uchar | 12.7ms | 12.2ms | 1.0x |
| 🟡 | bench/cstddef | 12.1ms | 10.2ms | 1.2x |
| 🟡 | bench/cstdint | 11.7ms | 10.7ms | 1.1x |
