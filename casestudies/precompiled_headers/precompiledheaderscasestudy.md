# Case study: precompiled headers vs. psychicstd

**Question:** a precompiled header (PCH) caches the parsed-and-instantiated
state of a header so it is paid once instead of on every translation unit. How
much does that help, and — since psychicstd attacks the same frontend cost by
shipping *less* to parse and instantiate — does psychicstd still win once PCH is
in play?

This is scoped to `<string>` only, which is the most complete part of
psychicstd, so the comparison against libstdc++ measures PCH rather than
psychicstd's missing features.

## Method

For each standard library (psychicstd and the system libstdc++) we measure:

- **baseline** — compile `tu.cpp` normally (parses + instantiates `<string>`
  anew every time);
- **pch-build** — the one-time cost of building the PCH from `pch.h`
  (which just `#include <string>`);
- **with-pch** — compile `tu.cpp` with `-include-pch`, reusing the cached
  header.

`tu.cpp` is a small program that exercises `std::string` (`find`, `substr`,
`operator+`, `compare`, …) so real instantiation happens.

Reproduce with:

```bash
./run_experiment.py
```

Environment: clang 21.1.8, `-std=c++20`, Linux x86-64, 12 reps per number
(median), warm cache.

## Results

| Config | baseline | pch-build (once) | with-pch | PCH size |
| --- | --- | --- | --- | --- |
| system libstdc++ | 215.6 ms | 234.6 ms | 71.9 ms | 4325 KB |
| psychicstd | 69.7 ms | 78.9 ms | 28.1 ms | 1480 KB |

Derived:

- **PCH speedup per TU:** system 3.0×, psychicstd 2.5×.
- **psychicstd vs system:** 3.1× without PCH → 2.6× with PCH (28.1 vs 71.9 ms).
- **Break-even:** the PCH pays for its one-time build after ~1.6 TUs (system)
  and ~1.9 TUs (psychicstd).

## Analysis

**PCH is a big win, and it stacks with psychicstd rather than replacing it.**
Both configs get ~2.5–3× faster per TU, because PCH skips re-parsing the whole
`<string>` header on every file. But psychicstd still compiles **2.6× faster
than libstdc++ even when both use a PCH** (28.1 vs 71.9 ms). Two reasons:

1. **Smaller PCH loads faster.** psychicstd's PCH is 1.5 MB vs libstdc++'s
   4.3 MB. `with-pch` isn't free — the compiler must deserialize the cached AST,
   and that cost scales with size. Less code in → smaller PCH → faster load.
1. **Less residual instantiation and codegen.** What PCH *doesn't* cache is the
   instantiation and codegen driven by the TU itself; psychicstd's simpler
   `std::string` leaves less of that behind (consistent with the
   [`<string>` study](../std_string/stdstringcasestudy.md), where libstdc++ spent
   far more in `InstantiateFunction`/`InstantiateClass`).

**The headline comparison:** psychicstd with **no** PCH (69.7 ms) is about as
fast as libstdc++ with a **fully warmed** PCH (71.9 ms). In other words,
psychicstd gives you PCH-class compile speed for `<string>` *without any
build-system changes* — no prefix header to maintain, no `.pch` to keep in sync
with flags, no stale-PCH rebuilds.

**Break-even is essentially immediate** (~2 TUs), so if you are already set up
for PCH it is worth using with either library. But the two are complementary,
not either/or: the lowest time here — 28 ms — comes from using psychicstd *and*
a PCH together.

**Takeaway:** PCH and psychicstd optimize the same frontend bottleneck by
different means (cache it once vs. have less of it), and combine well. psychicstd
delivers most of a PCH's benefit with zero setup, and still comes out ahead when
PCH is added on top — because a smaller library makes a smaller, faster-loading
PCH.

### Caveats

- Scoped to `<string>`; a real prefix header bundles many headers, where
  psychicstd's incomplete coverage would confound the comparison.
- Measures warm-cache, single-machine wall time. PCH also interacts with build
  parallelism and incremental rebuilds, not modeled here.

## Files

- [`pch.h`](pch.h) — the prefix header that gets precompiled
- [`tu.cpp`](tu.cpp) — the translation unit that uses `std::string`
- [`run_experiment.py`](run_experiment.py) — reproduces every number above
