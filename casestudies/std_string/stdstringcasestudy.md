# Case study: `<string>` compilation speed

A small experiment measuring *why* psychicstd compiles faster than the system
standard library (libstdc++), using a trivial but realistic `std::string`
program and clang's `-ftime-trace` profiler.

## The test program

A minimal function that takes a `std::string` by const reference, checks whether
it contains the word `"banana"`, and returns one string or another
([`example.cpp`](example.cpp)):

```cpp
#include <string>

std::string check(const std::string& s) {
  if (s.find("banana") != std::string::npos) {
    return "yummy";
  }
  return ":(";
}
```

## How to reproduce

```bash
./measure.sh
```

The script compiles the program 10 times each with psychicstd and with the
system libstdc++, reports median compile time, the size of the preprocessed
translation unit, and a `-ftime-trace` breakdown of the frontend.

Environment: clang 21.1.8, `-std=c++20`, Linux x86-64. Compiled with
`-c example.cpp -o /dev/null` (no real object file written).

## Results

### Compile time

| | psychicstd | system libstdc++ | speedup |
| --- | --- | --- | --- |
| Wall time (median of 10) | **60 ms** | 200 ms | **3.3×** |
| `ExecuteCompiler` (`-ftime-trace`) | 54 ms | 209 ms | 3.9× |

### Where the time goes

For this trivial function the backend (optimization + codegen) is negligible;
**almost all the time is the frontend** — parsing headers and instantiating
templates. The `-ftime-trace` category totals (milliseconds):

| Category | psychicstd | system | notes |
| --- | --- | --- | --- |
| ExecuteCompiler | 54.1 | 208.6 | whole compilation |
| Frontend | 49.1 | 197.4 | parse + sema |
| Source | 47.0 | 175.9 | parsing included files |
| ParseClass | 18.6 | 73.6 | parsing class definitions |
| InstantiateFunction | 0.5 | **29.9** | template function instantiation |
| InstantiateClass | 0.4 | **20.9** | template class instantiation |
| PerformPendingInstantiations | 0.4 | **16.6** | deferred instantiation |
| Backend | 3.5 | 9.1 | optimization + codegen |

### How much code the parser sees

| | distinct files parsed | non-blank lines | bytes |
| --- | --- | --- | --- |
| psychicstd | 23 | 6,356 | 286 KB |
| system libstdc++ | 61 | 21,692 | 857 KB |

## Analysis

Two effects explain the ~3× win, and both flow directly from psychicstd's design
goals (compile speed over runtime performance, no ABI/portability constraints):

1. **Far less code to parse.** Including `<string>` transitively pulls in ~3×
   less source with psychicstd (23 files / 286 KB vs 61 files / 857 KB). Parse
   time (`Source`, `ParseClass`) scales almost linearly with that volume, and it
   is the single largest cost in both configurations.

1. **Almost no template instantiation.** libstdc++ spends ~67 ms combined in
   `InstantiateFunction` + `InstantiateClass` + `PerformPendingInstantiations`,
   standing up the machinery behind `basic_string`, `char_traits`, allocators,
   and their supporting traits. psychicstd spends essentially **zero** here.
   This is the payoff of the "keep it simple" choices — e.g. no small-string
   optimization and minimal template metaprogramming — which leave the compiler
   with far less to instantiate.

The backend difference (3.5 ms vs 9.1 ms) is real but small in absolute terms;
it would matter more for optimization-heavy code, but psychicstd explicitly does
not target release-mode compile speed.

**Takeaway:** the speedup comes from feeding the frontend less work — fewer
header lines to parse and far fewer templates to instantiate — not from anything
in code generation.

## Files

- [`example.cpp`](example.cpp) — the test program
- [`measure.sh`](measure.sh) — reproduces every number above
