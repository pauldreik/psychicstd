# Compliance

Last updated: 2026-06-29 23:38

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
| 🟡 | `algorithm` | 🟡 29/276/314/314 | 576 ms | 79 ms | 🟢 7.3× | 897 |
| 🟡 | `array` | 🟡 26/28/40/40 | 95 ms | 61 ms | 🟢 1.6× | 265 |
| 🟡 | `cassert` | 🟡 1/6/9/9 | 45 ms | 32 ms | 🟢 1.4× | 2 |
| 🟡 | `cctype` | 🟡 2/4/5/5 | 41 ms | 34 ms | 🟡 1.2× | 19 |
| 🟡 | `cerrno` | 🟡 4/7/29/29 | 51 ms | 30 ms | 🟢 1.7× | 2 |
| 🟡 | `cfloat` | 🟡 4/7/29/29 | 36 ms | 31 ms | 🟡 1.2× | 2 |
| 🟡 | `chrono` | 🟡 40/356/408/408 | 547 ms | 37 ms | 🟢 14.9× | 244 |
| 🟡 | `ciso646` | 🟡 4/7/29/29 | 42 ms | 30 ms | 🟢 1.4× | 2 |
| 🟢 | `climits` | 🟢 38/38/115/115 | 39 ms | 36 ms | 🟡 1.1× | 2 |
| 🟡 | `cmath` | 🟡 1/3/12/12 | 378 ms | 43 ms | 🟢 8.7× | 119 |
| 🟡 | `compare` | 🟡 1/8/14/14 | 54 ms | 43 ms | 🟢 1.3× | 247 |
| 🟡 | `complex` | 🟡 1/71/86/86 | 489 ms | 27 ms | 🟢 18.2× | 25 |
| 🟡 | `concepts` | 🟡 1/4/35/35 | 72 ms | 194 ms | 🔴 0.4× | 255 |
| 🟢 | `cstddef` | 🟢 13/13/21/21 | 40 ms | 32 ms | 🟢 1.2× | 41 |
| 🟢 | `cstdint` | 🟢 3/3/5/5 | 50 ms | 31 ms | 🟢 1.6× | 39 |
| 🟡 | `cstdio` | 🟡 4/7/29/29 | 33 ms | 32 ms | 🟡 1.0× | 49 |
| 🟡 | `cstdlib` | 🟡 1/6/9/9 | 39 ms | 33 ms | 🟡 1.2× | 45 |
| 🟡 | `cstring` | 🟡 2/4/5/5 | 43 ms | 35 ms | 🟢 1.2× | 29 |
| 🟡 | `ctime` | 🟡 4/7/29/29 | 34 ms | 31 ms | 🟡 1.1× | 17 |
| 🟡 | `cwchar` | 🟡 4/7/29/29 | 33 ms | 33 ms | 🟡 1.0× | 57 |
| 🟡 | `deque` | 🟡 3/60/75/75 | 422 ms | 41 ms | 🟢 10.2× | 19 |
| 🟡 | `exception` | 🟡 9/21/21/21 | 65 ms | 49 ms | 🟢 1.3× | 109 |
| 🟡 | `fstream` | 🟡 5/33/65/65 | 471 ms | 94 ms | 🟢 5.0× | 292 |
| 🟡 | `functional` | 🟡 29/123/152/152 | 165 ms | 37 ms | 🟢 4.4× | 245 |
| ⬜ | `initializer_list` | ⬜ 0/0/0/0 | n/a | n/a | ⬜ n/a | 37 |
| 🟡 | `iomanip` | 🟡 6/123/129/129 | 467 ms | 90 ms | 🟢 5.2× | 82 |
| 🔴 | `iostream` | 🔴 0/1/1/1 | n/a | n/a | ⬜ n/a | 53 |
| 🟡 | `iterator` | 🟡 23/207/294/294 | 264 ms | 32 ms | 🟢 8.1× | 323 |
| 🟢 | `limits` | 🟢 36/36/37/37 | 41 ms | 38 ms | 🟡 1.1× | 320 |
| 🟡 | `list` | 🟡 12/67/86/86 | 323 ms | 66 ms | 🟢 4.9× | 261 |
| 🟡 | `map` | 🟡 54/73/86/86 | 341 ms | 104 ms | 🟢 3.3× | 527 |
| 🟡 | `memory` | 🟡 32/156/188/188 | 258 ms | 50 ms | 🟢 5.1× | 526 |
| 🟡 | `new` | 🟡 21/49/52/52 | 58 ms | 53 ms | 🟡 1.1× | 28 |
| 🟡 | `numeric` | 🟡 1/29/43/43 | 366 ms | 38 ms | 🟢 9.7× | 109 |
| 🟡 | `optional` | 🟡 14/68/79/79 | 90 ms | 61 ms | 🟢 1.5× | 166 |
| 🟡 | `ostream` | 🟡 3/52/53/53 | 462 ms | 90 ms | 🟢 5.1× | 348 |
| 🟡 | `random` | 🟡 19/447/486/486 | 265 ms | 54 ms | 🟢 4.9× | 325 |
| 🔴 | `ranges` | 🔴 0/402/581/581 | n/a | n/a | ⬜ n/a | 155 |
| 🟡 | `ratio` | 🟡 7/13/13/13 | 43 ms | 32 ms | 🟢 1.4× | 71 |
| 🟡 | `regex` | 🟡 13/146/171/171 | 1049 ms | 28 ms | 🟢 37.6× | 72 |
| 🟡 | `set` | 🟡 16/60/69/69 | 358 ms | 102 ms | 🟢 3.5× | 195 |
| 🟡 | `sstream` | 🟡 5/57/87/87 | 496 ms | 99 ms | 🟢 5.0× | 204 |
| 🟡 | `stack` | 🟡 16/36/36/36 | 319 ms | 82 ms | 🟢 3.9× | 39 |
| 🟢 | `stdexcept` | 🟢 9/9/9/9 | 187 ms | 82 ms | 🟢 2.3× | 69 |
| 🟡 | `streambuf` | 🟡 4/35/37/37 | 189 ms | 33 ms | 🟢 5.8× | 98 |
| 🟡 | `string` | 🟡 5/26/225/225 | 226 ms | 61 ms | 🟢 3.7× | 910 |
| 🟡 | `string_view` | 🟡 5/79/88/88 | 183 ms | 46 ms | 🟢 4.0× | 154 |
| 🟡 | `thread` | 🟡 13/271/318/318 | 522 ms | 28 ms | 🟢 18.6× | 21 |
| 🟡 | `tuple` | 🟡 7/75/89/89 | 256 ms | 52 ms | 🟢 5.0× | 254 |
| 🟡 | `type_traits` | 🟡 73/134/149/149 | 48 ms | 40 ms | 🟢 1.2× | 797 |
| 🔴 | `typeinfo` | 🔴 0/5/5/5 | n/a | n/a | ⬜ n/a | 34 |
| 🟡 | `unordered_map` | 🟡 9/83/111/111 | 457 ms | 111 ms | 🟢 4.1× | 215 |
| 🟡 | `unordered_set` | 🟡 1/76/97/97 | 432 ms | 41 ms | 🟢 10.5× | 37 |
| 🟡 | `utility` | 🟡 17/129/153/153 | 211 ms | 34 ms | 🟢 6.2× | 142 |
| 🟡 | `vector` | 🟡 53/57/76/76 | 383 ms | 111 ms | 🟢 3.4× | 891 |
