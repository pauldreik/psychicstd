# Compliance

Last updated: 2026-07-07 06:59

## Conformance

Conformance is shown as **x/y/z/w** where:

- **x** — tests that pass with psychicstd
- **y** — tests where the system STL (libstdc++) passes (only these are run against psychicstd; the rest are excluded as libc++-specific)
- **z** — total tests run so far for this header (grows with each incremental run)
- **w** — total eligible tests available for that header in the LLVM suite

🟢 all sampled tests pass 🟡 at least one test compiles (but not all pass) 🔴 nothing compiles

## Compilation speed

One libcxx test file (the first that passes the system STL) is compiled once with each STL. The speedup is the ratio of system time to psychicstd time — higher is better. n/a means no test file compiled successfully with the system STL in the sample, so no timing was available.

🟢 >1.2× 🟡 0.8×–1.2× 🔴 \<0.8×

## Results

| | header | conformance | system | psychicstd | speedup | lines |
|--|--------|------------|-------:|----------:|--------:|------:|
| 🟡 | `algorithm` | 🟡 34/247/314/314 | 790 ms | 119 ms | 🟢 6.6× | 1067 |
| 🟡 | `any` | 🟡 2/18/18/18 | 367 ms | 78 ms | 🟢 4.7× | 174 |
| 🟡 | `array` | 🟡 26/30/40/40 | 121 ms | 63 ms | 🟢 1.9× | 268 |
| 🟡 | `atomic` | 🟡 14/64/113/113 | 134 ms | 50 ms | 🟢 2.7× | 96 |
| 🟡 | `cassert` | 🟡 2/6/9/9 | 48 ms | 36 ms | 🟢 1.4× | 2 |
| 🟡 | `cctype` | 🟡 2/4/5/5 | 49 ms | 42 ms | 🟡 1.2× | 19 |
| 🟡 | `cerrno` | 🟡 4/7/29/29 | 51 ms | 40 ms | 🟢 1.3× | 2 |
| 🟡 | `cfloat` | 🟡 4/7/29/29 | 55 ms | 40 ms | 🟢 1.4× | 2 |
| 🟡 | `chrono` | 🟡 43/352/408/408 | 826 ms | 54 ms | 🟢 15.4× | 268 |
| 🟡 | `ciso646` | 🟡 4/7/29/29 | 54 ms | 37 ms | 🟢 1.5× | 2 |
| 🟢 | `climits` | 🟢 38/38/115/115 | 62 ms | 59 ms | 🟡 1.0× | 2 |
| 🟡 | `clocale` | 🟡 4/7/29/29 | 55 ms | 39 ms | 🟢 1.4× | 8 |
| 🟡 | `cmath` | 🟡 1/3/12/12 | 462 ms | 52 ms | 🟢 9.0× | 133 |
| 🟡 | `compare` | 🟡 1/8/14/14 | 67 ms | 56 ms | 🟡 1.2× | 249 |
| 🟡 | `complex` | 🟡 33/71/86/86 | 693 ms | 77 ms | 🟢 9.0× | 159 |
| 🟡 | `concepts` | 🟡 1/4/35/35 | 104 ms | 252 ms | 🔴 0.4× | 255 |
| 🟡 | `csignal` | 🟡 4/7/29/29 | 56 ms | 39 ms | 🟢 1.4× | 2 |
| 🟢 | `cstddef` | 🟢 13/13/21/21 | 55 ms | 42 ms | 🟢 1.3× | 41 |
| 🟢 | `cstdint` | 🟢 3/3/5/5 | 60 ms | 37 ms | 🟢 1.6× | 39 |
| 🟡 | `cstdio` | 🟡 4/7/29/29 | 52 ms | 38 ms | 🟢 1.4× | 49 |
| 🟡 | `cstdlib` | 🟡 2/6/9/9 | 49 ms | 37 ms | 🟢 1.3× | 45 |
| 🟡 | `cstring` | 🟡 2/4/5/5 | 48 ms | 41 ms | 🟡 1.2× | 29 |
| 🟡 | `ctime` | 🟡 4/7/29/29 | 54 ms | 41 ms | 🟢 1.3× | 17 |
| 🟡 | `cwchar` | 🟡 4/7/29/29 | 51 ms | 42 ms | 🟡 1.2× | 57 |
| 🟡 | `deque` | 🟡 22/55/75/75 | 870 ms | 138 ms | 🟢 6.3× | 121 |
| 🟡 | `exception` | 🟡 9/21/21/21 | 116 ms | 70 ms | 🟢 1.6× | 109 |
| 🟡 | `forward_list` | 🟡 13/64/83/83 | 738 ms | 95 ms | 🟢 7.8× | 165 |
| 🟡 | `fstream` | 🟡 7/33/65/65 | 665 ms | 146 ms | 🟢 4.6× | 258 |
| 🟡 | `functional` | 🟡 29/123/152/152 | 258 ms | 60 ms | 🟢 4.3× | 351 |
| ⬜ | `initializer_list` | ⬜ 0/0/0/0 | n/a | n/a | ⬜ n/a | 37 |
| 🟡 | `iomanip` | 🟡 39/123/129/129 | 671 ms | 140 ms | 🟢 4.8× | 82 |
| 🔴 | `iostream` | 🔴 0/1/1/1 | n/a | n/a | ⬜ n/a | 60 |
| 🟡 | `istream` | 🟡 6/56/58/58 | 642 ms | 100 ms | 🟢 6.4× | 337 |
| 🟡 | `iterator` | 🟡 40/207/294/294 | 388 ms | 62 ms | 🟢 6.2× | 632 |
| 🟢 | `limits` | 🟢 36/36/37/37 | 61 ms | 58 ms | 🟡 1.1× | 320 |
| 🟡 | `list` | 🟡 6/12/86/86 | 620 ms | 84 ms | 🟢 7.3× | 454 |
| 🟡 | `map` | 🟡 22/26/86/86 | 384 ms | 119 ms | 🟢 3.2× | 702 |
| 🟡 | `memory` | 🟡 33/152/188/188 | 685 ms | 67 ms | 🟢 10.2× | 588 |
| 🟡 | `mutex` | 🟡 7/87/108/108 | 747 ms | 50 ms | 🟢 14.9× | 87 |
| 🟡 | `new` | 🟡 21/49/52/52 | 89 ms | 73 ms | 🟢 1.2× | 28 |
| 🟡 | `numeric` | 🟡 1/29/43/43 | 513 ms | 66 ms | 🟢 7.8× | 109 |
| 🟡 | `optional` | 🟡 15/66/79/79 | 146 ms | 59 ms | 🟢 2.5× | 167 |
| 🟡 | `ostream` | 🟡 32/52/53/53 | 632 ms | 144 ms | 🟢 4.4× | 436 |
| 🟡 | `random` | 🟡 27/446/486/486 | 381 ms | 101 ms | 🟢 3.8× | 417 |
| 🔴 | `ranges` | 🔴 0/397/581/581 | n/a | n/a | ⬜ n/a | 155 |
| 🟡 | `ratio` | 🟡 7/13/13/13 | 54 ms | 41 ms | 🟢 1.3× | 71 |
| 🟡 | `regex` | 🟡 13/146/171/171 | 1478 ms | 45 ms | 🟢 32.8× | 133 |
| 🟡 | `set` | 🟡 3/23/69/69 | 602 ms | 67 ms | 🟢 9.0× | 204 |
| 🟡 | `sstream` | 🟡 12/57/87/87 | 747 ms | 180 ms | 🟢 4.2× | 222 |
| 🟡 | `stack` | 🟡 16/33/36/36 | 295 ms | 86 ms | 🟢 3.4× | 39 |
| 🟢 | `stdexcept` | 🟢 9/9/9/9 | 236 ms | 106 ms | 🟢 2.2× | 6 |
| 🟡 | `streambuf` | 🟡 24/35/37/37 | 266 ms | 128 ms | 🟢 2.1× | 167 |
| 🟡 | `string` | 🟡 5/26/225/225 | 487 ms | 101 ms | 🟢 4.8× | 967 |
| 🟡 | `string_view` | 🟡 6/79/88/88 | 259 ms | 79 ms | 🟢 3.3× | 145 |
| 🟡 | `thread` | 🟡 29/269/318/318 | 752 ms | 52 ms | 🟢 14.4× | 80 |
| 🟡 | `tuple` | 🟡 7/74/89/89 | 561 ms | 63 ms | 🟢 9.0× | 318 |
| 🟡 | `type_traits` | 🟡 74/134/149/149 | 62 ms | 50 ms | 🟢 1.2× | 820 |
| 🔴 | `typeinfo` | 🔴 0/5/5/5 | n/a | n/a | ⬜ n/a | 34 |
| 🟡 | `unordered_map` | 🟡 15/84/111/111 | 890 ms | 148 ms | 🟢 6.0× | 562 |
| 🟡 | `unordered_set` | 🟡 15/73/97/97 | 860 ms | 124 ms | 🟢 6.9× | 454 |
| 🟡 | `utility` | 🟡 20/127/153/153 | 313 ms | 59 ms | 🟢 5.3× | 147 |
| 🟡 | `valarray` | 🟡 8/206/207/207 | 331 ms | 81 ms | 🟢 4.1× | 92 |
| 🟢 | `vector` | 🟢 53/53/76/76 | 863 ms | 189 ms | 🟢 4.6× | 947 |
