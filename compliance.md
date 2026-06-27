# Compliance

Last updated: 2026-06-28 14:18

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
| 🟡 | `algorithm` | 🟡 29/276/314/314 | n/a | n/a | ⬜ n/a | 791 |
| 🟡 | `array` | 🟡 26/28/40/40 | 54 ms | 43 ms | 🟢 1.3× | 265 |
| 🟡 | `cassert` | 🟡 1/6/9/9 | n/a | n/a | ⬜ n/a | 2 |
| 🟡 | `cctype` | 🟡 2/4/5/5 | 25 ms | 16 ms | 🟢 1.6× | 19 |
| 🟢 | `climits` | 🟢 38/38/115/115 | 26 ms | 21 ms | 🟢 1.3× | 2 |
| 🟡 | `cmath` | 🟡 1/3/12/12 | n/a | n/a | ⬜ n/a | 113 |
| 🟢 | `cstddef` | 🟢 13/13/21/21 | 21 ms | 14 ms | 🟢 1.5× | 41 |
| 🟢 | `cstdint` | 🟢 3/3/5/5 | 36 ms | 15 ms | 🟢 2.5× | 39 |
| 🟡 | `cstdlib` | 🟡 1/6/9/9 | n/a | n/a | ⬜ n/a | 45 |
| 🟡 | `cstring` | 🟡 2/4/5/5 | 23 ms | 19 ms | 🟢 1.2× | 29 |
| 🟡 | `exception` | 🟡 9/21/21/21 | n/a | n/a | ⬜ n/a | 102 |
| 🟡 | `functional` | 🟡 29/123/152/152 | n/a | n/a | ⬜ n/a | 245 |
| 🟡 | `iterator` | 🟡 21/208/294/294 | n/a | n/a | ⬜ n/a | 304 |
| 🟢 | `limits` | 🟢 36/36/37/37 | 496 ms | 42 ms | 🟢 11.9× | 320 |
| 🟡 | `map` | 🟡 54/73/86/86 | 374 ms | 92 ms | 🟢 4.1× | 527 |
| 🟡 | `memory` | 🟡 30/156/188/188 | n/a | n/a | ⬜ n/a | 440 |
| 🟡 | `numeric` | 🟡 1/29/43/43 | n/a | n/a | ⬜ n/a | 109 |
| 🟡 | `optional` | 🟡 14/68/79/79 | n/a | n/a | ⬜ n/a | 166 |
| 🟡 | `random` | 🟡 19/447/486/486 | n/a | n/a | ⬜ n/a | 298 |
| 🟡 | `set` | 🟡 16/60/69/69 | n/a | n/a | ⬜ n/a | 195 |
| 🟢 | `stdexcept` | 🟢 9/9/9/9 | 168 ms | 58 ms | 🟢 2.9× | 69 |
| 🟡 | `string` | 🟡 5/26/225/225 | n/a | n/a | ⬜ n/a | 834 |
| 🟡 | `string_view` | 🟡 4/79/88/88 | n/a | n/a | ⬜ n/a | 150 |
| 🟡 | `tuple` | 🟡 5/75/89/89 | n/a | n/a | ⬜ n/a | 223 |
| 🟡 | `type_traits` | 🟡 73/134/149/149 | n/a | n/a | ⬜ n/a | 790 |
| 🟡 | `utility` | 🟡 17/129/153/153 | n/a | n/a | ⬜ n/a | 142 |
| 🟡 | `vector` | 🟡 53/57/76/76 | 386 ms | 102 ms | 🟢 3.8× | 889 |
