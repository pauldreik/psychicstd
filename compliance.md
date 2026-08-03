# Compliance

Last updated: 2026-08-03 21:32

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
| 🟡 | `algorithm` | 🟡 100/277/323/323 | 725 ms | 142 ms | 🟢 5.1× | 849 |
| 🟡 | `any` | 🟡 9/18/18/18 | 276 ms | 109 ms | 🟢 2.5× | 206 |
| 🟡 | `array` | 🟡 27/30/40/40 | 106 ms | 52 ms | 🟢 2.1× | 268 |
| 🟡 | `atomic` | 🟡 55/64/120/120 | 110 ms | 58 ms | 🟢 1.9× | 782 |
| 🟢 | `bit` | 🟢 2/2/15/15 | 114 ms | 76 ms | 🟢 1.5× | 87 |
| 🟡 | `bitset` | 🟡 10/40/45/45 | 490 ms | 97 ms | 🟢 5.0× | 203 |
| 🟡 | `cassert` | 🟡 3/6/9/9 | 41 ms | 31 ms | 🟢 1.3× | 2 |
| 🟡 | `cctype` | 🟡 2/4/5/5 | 41 ms | 33 ms | 🟢 1.2× | 19 |
| 🟡 | `cerrno` | 🟡 5/7/29/29 | 48 ms | 37 ms | 🟢 1.3× | 2 |
| 🟡 | `cfloat` | 🟡 5/7/29/29 | 48 ms | 35 ms | 🟢 1.4× | 2 |
| 🟡 | `chrono` | 🟡 62/352/408/408 | 664 ms | 52 ms | 🟢 12.8× | 238 |
| 🟡 | `ciso646` | 🟡 5/7/29/29 | 48 ms | 32 ms | 🟢 1.5× | 2 |
| 🟢 | `climits` | 🟢 38/38/115/115 | 51 ms | 45 ms | 🟡 1.1× | 2 |
| 🟡 | `clocale` | 🟡 5/7/29/29 | 43 ms | 32 ms | 🟢 1.3× | 8 |
| 🟡 | `cmath` | 🟡 1/3/19/19 | 386 ms | 39 ms | 🟢 9.9× | 257 |
| 🟡 | `compare` | 🟡 5/8/14/14 | 69 ms | 40 ms | 🟢 1.7× | 311 |
| 🟡 | `complex` | 🟡 42/71/86/86 | 565 ms | 66 ms | 🟢 8.5× | 268 |
| 🟡 | `concepts` | 🟡 2/4/35/35 | 84 ms | 159 ms | 🔴 0.5× | 255 |
| 🟡 | `condition_variable` | 🟡 13/26/26/26 | 757 ms | 82 ms | 🟢 9.2× | 164 |
| 🟡 | `csetjmp` | 🟡 3/6/9/9 | 38 ms | 35 ms | 🟡 1.1× | 7 |
| 🟡 | `csignal` | 🟡 5/7/29/29 | 56 ms | 35 ms | 🟢 1.6× | 7 |
| 🟢 | `cstddef` | 🟢 13/13/21/21 | 52 ms | 38 ms | 🟢 1.4× | 76 |
| 🟢 | `cstdint` | 🟢 3/3/5/5 | 45 ms | 31 ms | 🟢 1.4× | 39 |
| 🟡 | `cstdio` | 🟡 5/7/29/29 | 51 ms | 34 ms | 🟢 1.5× | 51 |
| 🟡 | `cstdlib` | 🟡 3/6/9/9 | 37 ms | 31 ms | 🟢 1.2× | 152 |
| 🟡 | `cstring` | 🟡 2/4/5/5 | 40 ms | 33 ms | 🟢 1.2× | 28 |
| 🟡 | `ctime` | 🟡 5/7/29/29 | 45 ms | 33 ms | 🟢 1.3× | 17 |
| 🟡 | `cwchar` | 🟡 5/7/29/29 | 44 ms | 38 ms | 🟡 1.2× | 57 |
| 🟡 | `deque` | 🟡 54/55/75/75 | 715 ms | 119 ms | 🟢 6.0× | 832 |
| 🟡 | `exception` | 🟡 20/21/21/21 | 142 ms | 62 ms | 🟢 2.3× | 237 |
| 🟡 | `filesystem` | 🟡 16/113/146/146 | 794 ms | 92 ms | 🟢 8.7× | 359 |
| 🟡 | `forward_list` | 🟡 15/64/83/83 | 595 ms | 80 ms | 🟢 7.5× | 177 |
| 🟡 | `fstream` | 🟡 19/33/65/65 | 523 ms | 127 ms | 🟢 4.1× | 308 |
| 🟡 | `functional` | 🟡 62/123/165/165 | 228 ms | 54 ms | 🟢 4.2× | 304 |
| ⬜ | `initializer_list` | ⬜ 0/0/0/0 | n/a | n/a | ⬜ n/a | 38 |
| 🟡 | `iomanip` | 🟡 79/123/129/129 | 532 ms | 119 ms | 🟢 4.5× | 179 |
| 🟡 | `ios` | 🟡 40/90/91/91 | 233 ms | 94 ms | 🟢 2.5× | 243 |
| 🔴 | `iostream` | 🔴 0/2/2/2 | n/a | n/a | ⬜ n/a | 18 |
| 🟡 | `istream` | 🟡 34/56/58/58 | 511 ms | 125 ms | 🟢 4.1× | 461 |
| 🟡 | `iterator` | 🟡 62/205/293/293 | 321 ms | 90 ms | 🟢 3.6× | 460 |
| 🟢 | `limits` | 🟢 36/36/37/37 | 54 ms | 47 ms | 🟡 1.1× | 293 |
| 🟡 | `list` | 🟡 8/12/86/86 | 515 ms | 66 ms | 🟢 7.8× | 432 |
| 🟡 | `locale` | 🟡 81/256/326/326 | 294 ms | 89 ms | 🟢 3.3× | 254 |
| 🟢 | `map` | 🟢 26/26/86/86 | 304 ms | 107 ms | 🟢 2.9× | 199 |
| 🟡 | `memory` | 🟡 63/148/196/196 | 624 ms | 73 ms | 🟢 8.5× | 789 |
| 🟡 | `mutex` | 🟡 35/87/108/108 | 656 ms | 86 ms | 🟢 7.6× | 183 |
| 🟡 | `new` | 🟡 26/49/52/52 | 70 ms | 63 ms | 🟡 1.1× | 42 |
| 🟡 | `numeric` | 🟡 4/29/45/45 | 471 ms | 128 ms | 🟢 3.7× | 136 |
| 🟡 | `optional` | 🟡 22/66/79/79 | 131 ms | 64 ms | 🟢 2.1× | 302 |
| 🟡 | `ostream` | 🟡 42/52/53/53 | 589 ms | 123 ms | 🟢 4.8× | 412 |
| 🟡 | `random` | 🟡 64/448/486/486 | 358 ms | 68 ms | 🟢 5.3× | 524 |
| 🟡 | `ranges` | 🟡 1/398/584/584 | 475 ms | 52 ms | 🟢 9.2× | 191 |
| 🟢 | `ratio` | 🟢 13/13/13/13 | 45 ms | 33 ms | 🟢 1.4× | 91 |
| 🟡 | `regex` | 🟡 14/146/171/171 | 1298 ms | 35 ms | 🟢 37.4× | 151 |
| 🟡 | `set` | 🟡 4/23/69/69 | 489 ms | 69 ms | 🟢 7.1× | 645 |
| 🟡 | `sstream` | 🟡 20/57/87/87 | 652 ms | 155 ms | 🟢 4.2× | 355 |
| 🟡 | `stack` | 🟡 16/33/36/36 | 246 ms | 75 ms | 🟢 3.3× | 45 |
| 🟢 | `stdexcept` | 🟢 9/9/9/9 | 203 ms | 80 ms | 🟢 2.5× | 6 |
| 🟡 | `streambuf` | 🟡 30/35/37/37 | 223 ms | 93 ms | 🟢 2.4× | 185 |
| 🟡 | `string` | 🟡 5/26/227/227 | 406 ms | 83 ms | 🟢 4.9× | 1170 |
| 🟡 | `string_view` | 🟡 27/79/88/88 | 217 ms | 57 ms | 🟢 3.8× | 351 |
| 🟡 | `system_error` | 🟡 43/59/60/60 | 215 ms | 82 ms | 🟢 2.6× | 19 |
| 🟡 | `thread` | 🟡 97/270/318/318 | 645 ms | 74 ms | 🟢 8.8× | 236 |
| 🟡 | `tuple` | 🟡 20/74/89/89 | 445 ms | 63 ms | 🟢 7.1× | 357 |
| 🟡 | `type_traits` | 🟡 88/134/149/149 | 52 ms | 42 ms | 🟢 1.2× | 1074 |
| 🟡 | `typeinfo` | 🟡 2/5/5/5 | 56 ms | 56 ms | 🟡 1.0× | 79 |
| 🟡 | `unordered_map` | 🟡 34/84/111/111 | 804 ms | 138 ms | 🟢 5.8× | 909 |
| 🟡 | `unordered_set` | 🟡 30/73/97/97 | 799 ms | 112 ms | 🟢 7.1× | 588 |
| 🟡 | `utility` | 🟡 28/127/153/153 | 247 ms | 49 ms | 🟢 5.0× | 229 |
| 🟡 | `valarray` | 🟡 8/206/207/207 | 295 ms | 59 ms | 🟢 5.0× | 92 |
| 🟢 | `vector` | 🟢 53/53/76/76 | 746 ms | 152 ms | 🟢 4.9× | 906 |
| ⬜ | `version` | ⬜ 0/0/76/76 | n/a | n/a | ⬜ n/a | 100 |
