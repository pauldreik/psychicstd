# Case study: C++20 modules (header units) vs. psychicstd

**Question:** C++20 lets you `import` a header as a *header unit* — the header is
compiled once into a binary module interface (BMI, a `.pcm`) and reused, instead
of being re-`#include`d and re-parsed per translation unit. This is the module
world's analog of a [precompiled header](../precompiled_headers/precompiledheaderscasestudy.md).
The hypothesis going in: modules will show the *same effect* as PCH — psychicstd
faster for the same reasons.

It turns out that is only half true, and the other half is the interesting part.

Scoped to `<string>` only (the most complete part of psychicstd) so the
comparison against libstdc++ is fair. Neither library is written as *named*
modules, so header units are the right, apples-to-apples comparison.

## Method

For each standard library we measure:

- **baseline** — compile `tu_include.cpp` normally (`#include <string>`);
- **hu-build** — the one-time cost of building the header unit from `pch.h`;
- **with-hu** — compile `tu_mod.cpp` (`import "pch.h";`) using the built BMI.

`tu_include.cpp` and `tu_mod.cpp` have identical bodies (exercise
`std::string`); they differ only in `#include` vs `import`.

Reproduce with:

```bash
./run_experiment.py
```

Environment: clang 21.1.8, `-std=c++20`, Linux x86-64, 15 reps per number
(median), warm cache.

## Results

| Config | baseline | hu-build (once) | with-hu | BMI size |
| --- | --- | --- | --- | --- |
| system libstdc++ | 216.5 ms | 249.4 ms | 44.7 ms | 4255 KB |
| psychicstd | 70.5 ms | 76.8 ms | 27.3 ms | 1224 KB |

Per-TU speedup from the header unit: **system 4.8×, psychicstd 2.6×**.
Break-even (build the BMI once, then compile K TUs): ~1.5 TUs (system),
~1.8 TUs (psychicstd).

### Side by side with the PCH study

| | baseline | + PCH | + module | tool artifact size |
| --- | --- | --- | --- | --- |
| system libstdc++ | 215 ms | 72 ms | **45 ms** | 4.3 / 4.3 MB |
| psychicstd | 69 ms | 28 ms | **27 ms** | 1.5 / 1.2 MB |
| **psychicstd advantage** | 3.1× | 2.6× | **1.65×** | |

## Analysis

**The hypothesis is confirmed for psychicstd.** A module (27 ms) is
essentially the same as a PCH (28 ms), and psychicstd still wins. The reason it
is fast is unchanged — there is simply less to load.

**But modules are *not* the same as PCH for libstdc++ — they are markedly
better** (45 ms vs 72 ms). As a result psychicstd's relative advantage shrinks
*more* under modules (1.65×) than under PCH (2.6×).

The cause is how the cached header is consumed:

- A **PCH is eagerly loaded** — the whole serialized AST is pulled in before the
  TU is compiled.
- A **header unit is lazily deserialized** — only the declarations the TU
  actually references are read out of the BMI.

libstdc++'s BMI is large (4.3 MB) and full of declarations this tiny TU never
touches; modules skip them, PCH cannot. psychicstd's BMI is small (1.2 MB), so
there is little unused material to skip — lazy and eager loading cost about the
same, hence module ≈ PCH for it.

**The deeper point:** modules automate, in the compiler, exactly what psychicstd
does by hand — *don't pay for code you don't use*. So the tool that most closes
the gap is the one whose entire job is to neutralize bloat. psychicstd's edge is
still real and comes for free (1.65× and no build-system changes, no BMI to keep
in sync with flags), but it is at its smallest precisely under modules — because
modules and psychicstd are, in a sense, two solutions to the same problem.

### Caveats

- Header units (`-fmodule-header`) are still an experimental/young feature in
  clang; this uses `-Wno-experimental-header-units`.
- Scoped to `<string>`; a real project imports many headers, where psychicstd's
  incomplete coverage would confound the comparison.
- Named modules (`export module …;` / `import std;`) are a different, larger
  undertaking not measured here — they would require modularizing the library
  itself.
- Warm-cache, single-machine wall time; build-graph and parallelism effects are
  not modeled.

## Files

- [`pch.h`](pch.h) — the header compiled into a header unit
- [`tu_include.cpp`](tu_include.cpp) — baseline TU (`#include`)
- [`tu_mod.cpp`](tu_mod.cpp) — module TU (`import`)
- [`run_experiment.py`](run_experiment.py) — reproduces every number above
