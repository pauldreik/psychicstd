# Compilation Speed

Median of 10 compilations per file. Ordered by system STL compile time, slowest first.

🟢 above 1.2x  🟡 0.8x–1.2x  🔴 below 0.8x

Last updated: 2026-07-16 00:51

Platform: macOS 26 arm64 (Apple M3), Apple clang 21, system STL = libc++.

| | name | system | psychicstd | speedup |
|--|------|-------:|----------:|--------:|
| 🟢 | wordcounter | 373.6ms | 62.4ms | 6.0x |
| 🟢 | regex | 348.5ms | 39.9ms | 8.7x |
| 🟢 | filesystem | 291.3ms | 59.6ms | 4.9x |
| 🟢 | thread | 265.3ms | 72.5ms | 3.7x |
| 🟢 | iostream | 260.5ms | 57.0ms | 4.6x |
| 🟢 | mutex | 247.9ms | 44.9ms | 5.5x |
| 🟢 | chrono | 246.4ms | 28.8ms | 8.5x |
| 🟢 | algorithm | 237.1ms | 41.4ms | 5.7x |
| 🟢 | atomic | 235.3ms | 43.7ms | 5.4x |
| 🟢 | ranges | 209.2ms | 28.7ms | 7.3x |
| 🟢 | queue | 207.0ms | 38.0ms | 5.5x |
| 🟢 | fstream | 206.6ms | 48.2ms | 4.3x |
| 🟢 | memory | 194.4ms | 44.7ms | 4.3x |
| 🟢 | vector | 191.2ms | 34.3ms | 5.6x |
| 🟢 | iomanip | 180.4ms | 54.0ms | 3.3x |
| 🟢 | sstream | 179.4ms | 51.8ms | 3.5x |
| 🟢 | iterator | 177.4ms | 28.1ms | 6.3x |
| 🟢 | locale | 174.4ms | 51.1ms | 3.4x |
| 🟢 | span | 174.4ms | 31.8ms | 5.5x |
| 🟢 | complex | 173.0ms | 23.2ms | 7.5x |
| 🟢 | istream | 170.2ms | 51.6ms | 3.3x |
| 🟢 | rj/write | 164.7ms | 96.7ms | 1.7x |
| 🟢 | rj/dom | 161.1ms | 93.0ms | 1.7x |
| 🟢 | random | 160.9ms | 38.1ms | 4.2x |
| 🟢 | rapidjson/write | 156.6ms | 93.1ms | 1.7x |
| 🟢 | variant | 156.2ms | 46.1ms | 3.4x |
| 🟢 | map | 151.2ms | 48.9ms | 3.1x |
| 🟢 | rapidjson/dom | 150.9ms | 88.0ms | 1.7x |
| 🟢 | rj/parse | 146.7ms | 79.4ms | 1.8x |
| 🟢 | rapidjson/parse | 138.1ms | 75.3ms | 1.8x |
| 🟢 | rapidjson | 137.2ms | 72.0ms | 1.9x |
| 🟢 | deque | 133.5ms | 33.1ms | 4.0x |
| 🟢 | functional | 124.7ms | 27.3ms | 4.6x |
| 🟢 | ios | 122.9ms | 39.0ms | 3.1x |
| 🟢 | bitset | 112.6ms | 22.3ms | 5.0x |
| 🟢 | string | 105.8ms | 36.4ms | 2.9x |
| 🟢 | stack | 105.6ms | 28.2ms | 3.7x |
| 🟢 | unordered_map | 102.7ms | 29.0ms | 3.5x |
| 🟢 | set | 102.4ms | 34.5ms | 3.0x |
| 🟢 | typeinfo | 100.9ms | 35.3ms | 2.9x |
| 🟢 | stdexcept | 97.7ms | 33.8ms | 2.9x |
| 🟢 | unordered_set | 96.3ms | 27.2ms | 3.5x |
| 🟢 | tuple | 90.6ms | 34.5ms | 2.6x |
| 🟢 | string_view | 78.2ms | 38.1ms | 2.1x |
| 🟢 | stop_token | 76.5ms | 33.6ms | 2.3x |
| 🟢 | list | 76.4ms | 24.6ms | 3.1x |
| 🟢 | forward_list | 74.7ms | 23.9ms | 3.1x |
| 🟢 | valarray | 73.9ms | 20.0ms | 3.7x |
| 🟢 | array | 60.1ms | 22.0ms | 2.7x |
| 🟢 | optional | 54.3ms | 27.4ms | 2.0x |
| 🟢 | any | 53.8ms | 26.9ms | 2.0x |
| 🟢 | utility | 37.8ms | 20.3ms | 1.9x |
| 🟢 | all-headers | 34.3ms | 21.1ms | 1.6x |
| 🟢 | numeric | 32.9ms | 20.7ms | 1.6x |
| 🟢 | numeric | 32.0ms | 20.1ms | 1.6x |
| 🟢 | cmath | 29.9ms | 23.4ms | 1.3x |
| 🟢 | compare | 27.9ms | 19.6ms | 1.4x |
| 🟢 | exception | 27.1ms | 18.6ms | 1.5x |
| 🟢 | new | 25.5ms | 18.6ms | 1.4x |
| 🟢 | type_traits | 24.8ms | 19.1ms | 1.3x |
| 🟡 | cstdlib | 24.5ms | 24.1ms | 1.0x |
| 🟡 | cstdlib_order | 23.9ms | 23.1ms | 1.0x |
| 🟡 | concepts | 21.6ms | 21.0ms | 1.0x |
| 🟡 | cstddef | 20.1ms | 18.0ms | 1.1x |
| 🟡 | ratio | 19.6ms | 20.4ms | 1.0x |
| 🟡 | limits | 19.4ms | 18.5ms | 1.1x |
| 🟡 | initializer_list | 18.2ms | 18.2ms | 1.0x |
| 🟡 | cstdint | 17.9ms | 17.3ms | 1.0x |
| 🟡 | cstddef | 17.9ms | 16.5ms | 1.1x |
| 🟡 | cstdint | 17.6ms | 17.1ms | 1.0x |
