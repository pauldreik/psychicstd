# Compilation Speed

Median of 10 compilations per file. Ordered by system STL compile time, slowest first.

🟢 above 1.2x  🟡 0.8x–1.2x  🔴 below 0.8x

Last updated: 2026-07-15 19:42

Platform: macOS 26 arm64 (Apple M3), Apple clang 21, system STL = libc++.

| | name | system | psychicstd | speedup |
|--|------|-------:|----------:|--------:|
| 🟢 | wordcounter | 415.1ms | 68.3ms | 6.1x |
| 🟢 | regex | 382.8ms | 48.9ms | 7.8x |
| 🟢 | filesystem | 313.8ms | 68.7ms | 4.6x |
| 🟢 | iostream | 288.1ms | 64.1ms | 4.5x |
| 🟢 | thread | 277.2ms | 76.4ms | 3.6x |
| 🟢 | mutex | 275.0ms | 54.0ms | 5.1x |
| 🟢 | chrono | 274.7ms | 38.5ms | 7.1x |
| 🟢 | algorithm | 261.1ms | 44.7ms | 5.8x |
| 🟢 | atomic | 242.9ms | 50.5ms | 4.8x |
| 🟢 | ranges | 228.1ms | 32.0ms | 7.1x |
| 🟢 | fstream | 226.1ms | 58.6ms | 3.9x |
| 🟢 | memory | 207.3ms | 52.6ms | 3.9x |
| 🟢 | vector | 206.3ms | 37.5ms | 5.5x |
| 🟢 | iomanip | 193.7ms | 60.5ms | 3.2x |
| 🟢 | sstream | 192.2ms | 58.3ms | 3.3x |
| 🟢 | complex | 191.5ms | 33.7ms | 5.7x |
| 🟢 | iterator | 181.1ms | 29.8ms | 6.1x |
| 🟢 | random | 178.4ms | 50.6ms | 3.5x |
| 🟢 | span | 178.3ms | 35.2ms | 5.1x |
| 🟢 | locale | 176.9ms | 57.4ms | 3.1x |
| 🟢 | istream | 174.5ms | 58.8ms | 3.0x |
| 🟢 | rapidjson/write | 167.2ms | 101.7ms | 1.6x |
| 🟢 | rj/write | 167.2ms | 101.3ms | 1.6x |
| 🟢 | variant | 166.7ms | 55.3ms | 3.0x |
| 🟢 | rj/dom | 165.8ms | 97.3ms | 1.7x |
| 🟢 | rapidjson/dom | 161.4ms | 96.2ms | 1.7x |
| 🟢 | rj/parse | 156.9ms | 86.7ms | 1.8x |
| 🟢 | map | 154.1ms | 50.7ms | 3.0x |
| 🟢 | rapidjson | 151.2ms | 82.1ms | 1.8x |
| 🟢 | rapidjson/parse | 149.4ms | 83.2ms | 1.8x |
| 🟢 | deque | 143.1ms | 35.9ms | 4.0x |
| 🟢 | functional | 136.8ms | 30.6ms | 4.5x |
| 🟢 | ios | 126.6ms | 46.0ms | 2.8x |
| 🟢 | bitset | 123.4ms | 25.3ms | 4.9x |
| 🟢 | set | 112.9ms | 37.5ms | 3.0x |
| 🟢 | string | 108.7ms | 43.2ms | 2.5x |
| 🟢 | stack | 107.5ms | 30.0ms | 3.6x |
| 🟢 | unordered_map | 105.5ms | 31.7ms | 3.3x |
| 🟢 | typeinfo | 104.1ms | 42.2ms | 2.5x |
| 🟢 | stdexcept | 99.9ms | 40.8ms | 2.4x |
| 🟢 | unordered_set | 98.2ms | 30.3ms | 3.2x |
| 🟢 | tuple | 94.4ms | 37.1ms | 2.5x |
| 🟢 | forward_list | 81.7ms | 26.9ms | 3.0x |
| 🟢 | string_view | 80.3ms | 45.1ms | 1.8x |
| 🟢 | list | 78.2ms | 26.6ms | 2.9x |
| 🟢 | stop_token | 77.8ms | 39.2ms | 2.0x |
| 🟢 | valarray | 75.4ms | 23.2ms | 3.2x |
| 🟢 | array | 62.4ms | 24.6ms | 2.5x |
| 🟢 | optional | 60.6ms | 30.0ms | 2.0x |
| 🟢 | any | 57.1ms | 29.1ms | 2.0x |
| 🟢 | utility | 39.1ms | 22.4ms | 1.7x |
| 🟢 | all-headers | 37.1ms | 24.3ms | 1.5x |
| 🟢 | numeric | 36.7ms | 23.6ms | 1.6x |
| 🟢 | numeric | 36.6ms | 23.5ms | 1.6x |
| 🟡 | cmath | 33.4ms | 32.7ms | 1.0x |
| 🟢 | compare | 31.7ms | 22.9ms | 1.4x |
| 🟢 | exception | 30.3ms | 21.3ms | 1.4x |
| 🟢 | new | 28.7ms | 21.9ms | 1.3x |
| 🟢 | type_traits | 27.1ms | 20.9ms | 1.3x |
| 🟡 | concepts | 25.2ms | 24.4ms | 1.0x |
| 🟡 | cstddef | 22.6ms | 21.0ms | 1.1x |
| 🟡 | ratio | 22.4ms | 23.4ms | 1.0x |
| 🟡 | initializer_list | 21.6ms | 20.8ms | 1.0x |
| 🟡 | limits | 21.3ms | 26.5ms | 0.8x |
| 🟡 | cstddef | 21.1ms | 19.2ms | 1.1x |
| 🟡 | cstdint | 20.4ms | 19.5ms | 1.0x |
| 🟡 | cstdint | 20.3ms | 20.3ms | 1.0x |
