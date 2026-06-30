# Compliance

Last updated: 2026-06-30 23:31

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
| 🟡 | `algorithm` | 🟡 32/276/314/314 | 593 ms | 84 ms | 🟢 7.0× | 1122 |
| 🟡 | `any` | 🟡 2/18/18/18 | 254 ms | 63 ms | 🟢 4.1× | 173 |
| 🟡 | `array` | 🟡 26/28/40/40 | 110 ms | 71 ms | 🟢 1.5× | 265 |
| 🟡 | `atomic` | 🟡 14/74/113/113 | 142 ms | 41 ms | 🟢 3.5× | 96 |
| 🟡 | `cassert` | 🟡 2/6/9/9 | 45 ms | 32 ms | 🟢 1.4× | 2 |
| 🟡 | `cctype` | 🟡 2/4/5/5 | 43 ms | 35 ms | 🟢 1.2× | 19 |
| 🟡 | `cerrno` | 🟡 4/7/29/29 | 51 ms | 37 ms | 🟢 1.4× | 2 |
| 🟡 | `cfloat` | 🟡 4/7/29/29 | 46 ms | 36 ms | 🟢 1.3× | 2 |
| 🟡 | `chrono` | 🟡 43/356/408/408 | 579 ms | 37 ms | 🟢 15.5× | 268 |
| 🟡 | `ciso646` | 🟡 4/7/29/29 | 41 ms | 39 ms | 🟡 1.1× | 2 |
| 🟢 | `climits` | 🟢 38/38/115/115 | 46 ms | 42 ms | 🟡 1.1× | 2 |
| 🟡 | `clocale` | 🟡 4/7/29/29 | 37 ms | 32 ms | 🟡 1.1× | 8 |
| 🟡 | `cmath` | 🟡 1/3/12/12 | 382 ms | 44 ms | 🟢 8.7× | 127 |
| 🟡 | `compare` | 🟡 1/8/14/14 | 59 ms | 46 ms | 🟢 1.3× | 249 |
| 🟡 | `complex` | 🟡 33/71/86/86 | 519 ms | 59 ms | 🟢 8.8× | 159 |
| 🟡 | `concepts` | 🟡 1/4/35/35 | 79 ms | 220 ms | 🔴 0.4× | 255 |
| 🟡 | `csignal` | 🟡 4/7/29/29 | 47 ms | 34 ms | 🟢 1.4× | 2 |
| 🟢 | `cstddef` | 🟢 13/13/21/21 | 42 ms | 35 ms | 🟢 1.2× | 41 |
| 🟢 | `cstdint` | 🟢 3/3/5/5 | 49 ms | 33 ms | 🟢 1.5× | 39 |
| 🟡 | `cstdio` | 🟡 4/7/29/29 | 40 ms | 32 ms | 🟢 1.2× | 49 |
| 🟡 | `cstdlib` | 🟡 2/6/9/9 | 46 ms | 32 ms | 🟢 1.5× | 45 |
| 🟡 | `cstring` | 🟡 2/4/5/5 | 42 ms | 35 ms | 🟢 1.2× | 29 |
| 🟡 | `ctime` | 🟡 4/7/29/29 | 48 ms | 33 ms | 🟢 1.5× | 17 |
| 🟡 | `cwchar` | 🟡 4/7/29/29 | 46 ms | 37 ms | 🟢 1.2× | 57 |
| 🟡 | `deque` | 🟡 22/60/75/75 | 423 ms | 107 ms | 🟢 3.9× | 121 |
| 🟡 | `exception` | 🟡 9/21/21/21 | 74 ms | 54 ms | 🟢 1.4× | 109 |
| 🟡 | `forward_list` | 🟡 13/70/83/83 | 352 ms | 72 ms | 🟢 4.9× | 165 |
| 🟡 | `fstream` | 🟡 6/33/65/65 | 502 ms | 122 ms | 🟢 4.1× | 258 |
| 🟡 | `functional` | 🟡 29/123/152/152 | 177 ms | 40 ms | 🟢 4.4× | 247 |
| ⬜ | `initializer_list` | ⬜ 0/0/0/0 | n/a | n/a | ⬜ n/a | 37 |
| 🟡 | `iomanip` | 🟡 7/123/129/129 | 493 ms | 99 ms | 🟢 5.0× | 82 |
| 🔴 | `iostream` | 🔴 0/1/1/1 | n/a | n/a | ⬜ n/a | 53 |
| 🟡 | `istream` | 🟡 3/56/58/58 | 503 ms | 72 ms | 🟢 7.0× | 370 |
| 🟡 | `iterator` | 🟡 33/208/294/294 | 279 ms | 54 ms | 🟢 5.1× | 594 |
| 🟢 | `limits` | 🟢 36/36/37/37 | 45 ms | 42 ms | 🟡 1.1× | 320 |
| 🟡 | `list` | 🟡 12/67/86/86 | 346 ms | 71 ms | 🟢 4.9× | 287 |
| 🟡 | `map` | 🟡 57/73/86/86 | 355 ms | 111 ms | 🟢 3.2× | 578 |
| 🟡 | `memory` | 🟡 32/156/188/188 | 272 ms | 54 ms | 🟢 5.1× | 588 |
| 🟡 | `mutex` | 🟡 7/88/108/108 | 574 ms | 32 ms | 🟢 17.7× | 87 |
| 🟡 | `new` | 🟡 21/49/52/52 | 66 ms | 57 ms | 🟡 1.2× | 28 |
| 🟡 | `numeric` | 🟡 1/29/43/43 | 404 ms | 36 ms | 🟢 11.1× | 109 |
| 🟡 | `optional` | 🟡 14/68/79/79 | 105 ms | 67 ms | 🟢 1.6× | 166 |
| 🟡 | `ostream` | 🟡 3/52/53/53 | 492 ms | 104 ms | 🟢 4.7× | 348 |
| 🟡 | `random` | 🟡 19/447/486/486 | 274 ms | 53 ms | 🟢 5.1× | 325 |
| 🔴 | `ranges` | 🔴 0/402/581/581 | n/a | n/a | ⬜ n/a | 155 |
| 🟡 | `ratio` | 🟡 7/13/13/13 | 49 ms | 35 ms | 🟢 1.4× | 71 |
| 🟡 | `regex` | 🟡 13/146/171/171 | 1039 ms | 31 ms | 🟢 34.0× | 132 |
| 🟡 | `set` | 🟡 18/60/69/69 | 368 ms | 107 ms | 🟢 3.4× | 204 |
| 🟡 | `sstream` | 🟡 11/57/87/87 | 534 ms | 130 ms | 🟢 4.1× | 217 |
| 🟡 | `stack` | 🟡 16/36/36/36 | 326 ms | 85 ms | 🟢 3.8× | 39 |
| 🟢 | `stdexcept` | 🟢 9/9/9/9 | 205 ms | 107 ms | 🟢 1.9× | 69 |
| 🟡 | `streambuf` | 🟡 4/35/37/37 | 198 ms | 32 ms | 🟢 6.2× | 150 |
| 🟡 | `string` | 🟡 5/26/225/225 | 236 ms | 67 ms | 🟢 3.5× | 993 |
| 🟡 | `string_view` | 🟡 6/79/88/88 | 187 ms | 49 ms | 🟢 3.8× | 154 |
| 🟡 | `thread` | 🟡 29/270/318/318 | 545 ms | 38 ms | 🟢 14.5× | 80 |
| 🟡 | `tuple` | 🟡 7/75/89/89 | 270 ms | 52 ms | 🟢 5.2× | 254 |
| 🟡 | `type_traits` | 🟡 74/134/149/149 | 48 ms | 40 ms | 🟡 1.2× | 802 |
| 🔴 | `typeinfo` | 🔴 0/5/5/5 | n/a | n/a | ⬜ n/a | 34 |
| 🟡 | `unordered_map` | 🟡 12/83/111/111 | 442 ms | 134 ms | 🟢 3.3× | 355 |
| 🟡 | `unordered_set` | 🟡 9/76/97/97 | 422 ms | 106 ms | 🟢 4.0× | 288 |
| 🟡 | `utility` | 🟡 19/129/153/153 | 215 ms | 43 ms | 🟢 5.0× | 144 |
| 🟡 | `valarray` | 🟡 8/206/207/207 | 232 ms | 57 ms | 🟢 4.1× | 92 |
| 🟡 | `vector` | 🟡 53/57/76/76 | 403 ms | 131 ms | 🟢 3.1× | 914 |
