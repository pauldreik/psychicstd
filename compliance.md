# Compliance

Last updated: 2026-06-28 18:00

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
| 🟡 | `algorithm` | 🟡 29/247/314/314 | n/a | n/a | ⬜ n/a | 898 |
| 🟡 | `array` | 🟡 26/30/40/40 | 135 ms | 108 ms | 🟢 1.3× | 265 |
| 🟡 | `cassert` | 🟡 1/6/9/9 | n/a | n/a | ⬜ n/a | 2 |
| 🟡 | `cctype` | 🟡 2/4/5/5 | 61 ms | 44 ms | 🟢 1.4× | 19 |
| 🟢 | `climits` | 🟢 38/38/115/115 | 1095 ms | 101 ms | 🟢 10.9× | 2 |
| 🟡 | `cmath` | 🟡 1/3/12/12 | n/a | n/a | ⬜ n/a | 119 |
| 🟢 | `cstddef` | 🟢 13/13/21/21 | 58 ms | 41 ms | 🟢 1.4× | 41 |
| 🟢 | `cstdint` | 🟢 3/3/5/5 | 336 ms | 117 ms | 🟢 2.9× | 39 |
| 🟡 | `cstdlib` | 🟡 1/6/9/9 | n/a | n/a | ⬜ n/a | 45 |
| 🟡 | `cstring` | 🟡 2/4/5/5 | 60 ms | 42 ms | 🟢 1.4× | 29 |
| 🟡 | `exception` | 🟡 9/21/21/21 | 68 ms | 44 ms | 🟢 1.5× | 108 |
| 🟡 | `functional` | 🟡 29/123/152/152 | 332 ms | 46 ms | 🟢 7.2× | 245 |
| 🟡 | `iterator` | 🟡 23/207/294/294 | n/a | n/a | ⬜ n/a | 323 |
| 🟢 | `limits` | 🟢 36/36/37/37 | 49 ms | 44 ms | 🟡 1.1× | 320 |
| 🟢 | `map` | 🟢 26/26/86/86 | 327 ms | 151 ms | 🟢 2.2× | 527 |
| 🟡 | `memory` | 🟡 30/152/188/188 | n/a | n/a | ⬜ n/a | 497 |
| 🟡 | `numeric` | 🟡 1/29/43/43 | n/a | n/a | ⬜ n/a | 109 |
| 🟡 | `optional` | 🟡 14/66/79/79 | n/a | n/a | ⬜ n/a | 166 |
| 🟡 | `random` | 🟡 19/446/486/486 | n/a | n/a | ⬜ n/a | 325 |
| 🟡 | `set` | 🟡 3/23/69/69 | n/a | n/a | ⬜ n/a | 195 |
| 🟢 | `stdexcept` | 🟢 9/9/9/9 | 383 ms | 121 ms | 🟢 3.2× | 69 |
| 🟡 | `string` | 🟡 5/26/225/225 | n/a | n/a | ⬜ n/a | 869 |
| 🟡 | `string_view` | 🟡 5/79/88/88 | n/a | n/a | ⬜ n/a | 150 |
| 🟡 | `tuple` | 🟡 7/74/89/89 | n/a | n/a | ⬜ n/a | 254 |
| 🟡 | `type_traits` | 🟡 73/134/149/149 | n/a | n/a | ⬜ n/a | 790 |
| 🟡 | `utility` | 🟡 17/127/153/153 | 78 ms | 39 ms | 🟢 2.0× | 142 |
| 🟢 | `vector` | 🟢 64/64/91/76 | 1808 ms | 364 ms | 🟢 5.0× | 889 |
