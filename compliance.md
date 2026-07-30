# Compliance

Last updated: 2026-07-30 10:27

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
| 🟡 | `algorithm` | 🟡 100/275/317/319 | 882 ms | 171 ms | 🟢 5.2× | 849 |
| 🟡 | `any` | 🟡 2/18/18/18 | 360 ms | 68 ms | 🟢 5.3× | 182 |
| 🟡 | `array` | 🟡 26/30/40/40 | 115 ms | 51 ms | 🟢 2.3× | 268 |
| 🟡 | `atomic` | 🟡 47/64/113/113 | 112 ms | 52 ms | 🟢 2.1× | 785 |
| 🟡 | `cassert` | 🟡 2/6/9/9 | 49 ms | 38 ms | 🟢 1.3× | 2 |
| 🟡 | `cctype` | 🟡 2/4/5/5 | 49 ms | 45 ms | 🟡 1.1× | 19 |
| 🟡 | `cerrno` | 🟡 4/7/29/29 | 50 ms | 37 ms | 🟢 1.3× | 2 |
| 🟡 | `cfloat` | 🟡 4/7/29/29 | 50 ms | 37 ms | 🟢 1.3× | 2 |
| 🟡 | `chrono` | 🟡 51/352/408/408 | 772 ms | 56 ms | 🟢 13.8× | 99 |
| 🟡 | `ciso646` | 🟡 4/7/29/29 | 50 ms | 40 ms | 🟢 1.2× | 2 |
| 🟢 | `climits` | 🟢 38/38/115/115 | 46 ms | 43 ms | 🟡 1.1× | 2 |
| 🟡 | `clocale` | 🟡 4/7/29/29 | 56 ms | 39 ms | 🟢 1.4× | 8 |
| 🟡 | `cmath` | 🟡 1/3/12/12 | 453 ms | 51 ms | 🟢 8.9× | 283 |
| 🟡 | `compare` | 🟡 5/8/14/14 | 68 ms | 47 ms | 🟢 1.5× | 311 |
| 🟡 | `complex` | 🟡 34/71/86/86 | 574 ms | 56 ms | 🟢 10.2× | 159 |
| 🟡 | `concepts` | 🟡 1/4/35/35 | 65 ms | 178 ms | 🔴 0.4× | 255 |
| 🟡 | `condition_variable` | 🟡 13/26/26/26 | 900 ms | 96 ms | 🟢 9.3× | 138 |
| 🟡 | `csetjmp` | 🟡 3/6/9/9 | 44 ms | 38 ms | 🟡 1.2× | 7 |
| 🟡 | `csignal` | 🟡 4/7/29/29 | 58 ms | 43 ms | 🟢 1.3× | 2 |
| 🟢 | `cstddef` | 🟢 13/13/21/21 | 55 ms | 35 ms | 🟢 1.6× | 41 |
| 🟢 | `cstdint` | 🟢 3/3/5/5 | 58 ms | 35 ms | 🟢 1.7× | 39 |
| 🟡 | `cstdio` | 🟡 4/7/29/29 | 53 ms | 43 ms | 🟢 1.2× | 51 |
| 🟡 | `cstdlib` | 🟡 2/6/9/9 | 46 ms | 35 ms | 🟢 1.3× | 45 |
| 🟡 | `cstring` | 🟡 2/4/5/5 | 50 ms | 46 ms | 🟡 1.1× | 29 |
| 🟡 | `ctime` | 🟡 4/7/29/29 | 53 ms | 44 ms | 🟡 1.2× | 17 |
| 🟡 | `cwchar` | 🟡 4/7/29/29 | 60 ms | 39 ms | 🟢 1.5× | 57 |
| 🟡 | `deque` | 🟡 54/55/75/75 | 833 ms | 146 ms | 🟢 5.7× | 832 |
| 🟡 | `exception` | 🟡 9/21/21/21 | 90 ms | 56 ms | 🟢 1.6× | 109 |
| 🟡 | `forward_list` | 🟡 13/64/83/83 | 607 ms | 66 ms | 🟢 9.1× | 165 |
| 🟡 | `fstream` | 🟡 7/33/65/65 | 535 ms | 153 ms | 🟢 3.5× | 258 |
| 🟡 | `functional` | 🟡 61/123/152/165 | 251 ms | 64 ms | 🟢 3.9× | 299 |
| ⬜ | `initializer_list` | ⬜ 0/0/0/0 | n/a | n/a | ⬜ n/a | 37 |
| 🟡 | `iomanip` | 🟡 44/123/129/129 | 540 ms | 168 ms | 🟢 3.2× | 103 |
| 🔴 | `iostream` | 🔴 0/1/1/1 | n/a | n/a | ⬜ n/a | 60 |
| 🟡 | `istream` | 🟡 34/56/58/58 | 597 ms | 149 ms | 🟢 4.0× | 458 |
| 🟡 | `iterator` | 🟡 62/205/293/293 | 361 ms | 99 ms | 🟢 3.6× | 460 |
| 🟢 | `limits` | 🟢 36/36/37/37 | 50 ms | 44 ms | 🟡 1.1× | 320 |
| 🟡 | `list` | 🟡 6/12/86/86 | 543 ms | 67 ms | 🟢 8.0× | 454 |
| 🟡 | `locale` | 🟡 81/256/326/326 | 326 ms | 104 ms | 🟢 3.1× | 254 |
| 🟢 | `map` | 🟢 26/26/86/86 | 330 ms | 105 ms | 🟢 3.2× | 208 |
| 🟡 | `memory` | 🟡 57/148/188/196 | 649 ms | 74 ms | 🟢 8.8× | 752 |
| 🟡 | `mutex` | 🟡 35/87/108/108 | 715 ms | 100 ms | 🟢 7.2× | 169 |
| 🟡 | `new` | 🟡 21/49/52/52 | 80 ms | 59 ms | 🟢 1.4× | 28 |
| 🟡 | `numeric` | 🟡 1/29/43/43 | 447 ms | 45 ms | 🟢 10.0× | 109 |
| 🟡 | `optional` | 🟡 15/66/79/79 | 111 ms | 45 ms | 🟢 2.5× | 174 |
| 🟡 | `ostream` | 🟡 36/52/53/53 | 552 ms | 184 ms | 🟢 3.0× | 464 |
| 🟡 | `random` | 🟡 28/450/486/486 | 300 ms | 75 ms | 🟢 4.0× | 417 |
| 🔴 | `ranges` | 🔴 0/398/581/581 | n/a | n/a | ⬜ n/a | 163 |
| 🟡 | `ratio` | 🟡 7/13/13/13 | 63 ms | 36 ms | 🟢 1.7× | 71 |
| 🟡 | `regex` | 🟡 13/146/171/171 | 2043 ms | 31 ms | 🟢 65.8× | 133 |
| 🟡 | `set` | 🟡 4/23/69/69 | 588 ms | 81 ms | 🟢 7.3× | 625 |
| 🟡 | `sstream` | 🟡 12/57/87/87 | 604 ms | 176 ms | 🟢 3.4× | 222 |
| 🟡 | `stack` | 🟡 16/33/36/36 | 236 ms | 73 ms | 🟢 3.2× | 45 |
| 🟢 | `stdexcept` | 🟢 9/9/9/9 | 215 ms | 107 ms | 🟢 2.0× | 6 |
| 🟡 | `streambuf` | 🟡 29/35/37/37 | 270 ms | 107 ms | 🟢 2.5× | 185 |
| 🟡 | `string` | 🟡 5/26/227/227 | 515 ms | 84 ms | 🟢 6.1× | 1156 |
| 🟡 | `string_view` | 🟡 26/79/88/88 | 263 ms | 74 ms | 🟢 3.5× | 339 |
| 🟡 | `system_error` | 🟡 41/59/60/60 | 252 ms | 89 ms | 🟢 2.8× | 19 |
| 🟡 | `thread` | 🟡 30/270/318/318 | 648 ms | 39 ms | 🟢 16.7× | 141 |
| 🟡 | `tuple` | 🟡 8/74/89/89 | 338 ms | 46 ms | 🟢 7.4× | 313 |
| 🟡 | `type_traits` | 🟡 75/134/149/149 | 47 ms | 40 ms | 🟡 1.2× | 831 |
| 🟡 | `typeinfo` | 🟡 1/5/5/5 | 64 ms | 54 ms | 🟡 1.2× | 38 |
| 🟡 | `unordered_map` | 🟡 34/84/111/111 | 817 ms | 136 ms | 🟢 6.0× | 892 |
| 🟡 | `unordered_set` | 🟡 30/73/97/97 | 816 ms | 121 ms | 🟢 6.8× | 587 |
| 🟡 | `utility` | 🟡 28/127/153/153 | 291 ms | 49 ms | 🟢 5.9× | 229 |
| 🟡 | `valarray` | 🟡 8/206/207/207 | 300 ms | 61 ms | 🟢 4.9× | 92 |
| 🟢 | `vector` | 🟢 53/53/76/76 | 772 ms | 144 ms | 🟢 5.4× | 906 |
