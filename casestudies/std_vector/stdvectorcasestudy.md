# Case study: `<vector>` compilation speed

A small experiment measuring *why* psychicstd compiles faster than the system
standard library (libstdc++), using a trivial but realistic `std::vector`
program and clang's `-ftime-trace` profiler. It mirrors the
[`<string>` case study](../std_string/stdstringcasestudy.md).

## The test program

A minimal function that takes a `std::vector<int>` by const reference and
returns a new vector containing only the even elements
([`example.cpp`](example.cpp)):

```cpp
#include <vector>

std::vector<int> evens(const std::vector<int>& v) {
  std::vector<int> result;
  for (int x : v) {
    if (x % 2 == 0) {
      result.push_back(x);
    }
  }
  return result;
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
| Wall time (median of 10) | **70 ms** | 170 ms | **2.4×** |
| `ExecuteCompiler` (`-ftime-trace`) | 70 ms | 173 ms | 2.5× |

### Where the time goes

As with `<string>`, the backend is negligible for this trivial function —
**almost all the time is the frontend**, parsing headers and instantiating
templates. The `-ftime-trace` category totals (milliseconds):

| Category | psychicstd | system | notes |
| --- | --- | --- | --- |
| ExecuteCompiler | 70.1 | 173.3 | whole compilation |
| Frontend | 63.3 | 162.8 | parse + sema |
| Source | 58.8 | 150.7 | parsing included files |
| ParseClass | 25.5 | 86.9 | parsing class definitions |
| InstantiateFunction | 2.1 | 11.1 | template function instantiation |
| InstantiateClass | 1.4 | **24.6** | template class instantiation |
| PerformPendingInstantiations | 0.3 | 0.3 | deferred instantiation |
| Backend | 5.3 | 8.5 | optimization + codegen |

### How much code the parser sees

| | distinct files parsed | non-blank lines | bytes |
| --- | --- | --- | --- |
| psychicstd | 28 | 7,209 | 312 KB |
| system libstdc++ | 45 | 19,077 | 713 KB |

## Analysis

The same two effects seen with `<string>` explain the win, and both flow
directly from psychicstd's design goals (compile speed over runtime
performance, no ABI/portability constraints):

1. **Less code to parse.** Including `<vector>` transitively pulls in ~2.3×
   less source with psychicstd (28 files / 312 KB vs 45 files / 713 KB). Parse
   time (`Source`, `ParseClass`) scales with that volume and is the single
   largest cost in both configurations.

1. **Far less template instantiation.** libstdc++ spends ~36 ms combined in
   `InstantiateClass` + `InstantiateFunction`, standing up `vector`, its
   iterators, allocator machinery, and supporting traits. psychicstd spends
   only ~3.5 ms. The gap is dominated by `InstantiateClass` (24.6 ms vs
   1.4 ms) — libstdc++'s `std::vector` drags in a much larger web of
   dependent class templates.

The backend difference (5.3 ms vs 8.5 ms) is real but small in absolute terms;
psychicstd explicitly does not target release-mode compile speed.

**Takeaway:** as with `<string>`, the speedup comes from feeding the frontend
less work — fewer header lines to parse and fewer templates to instantiate —
not from anything in code generation. The overall speedup is smaller here
(2.4× vs 3.3× for `<string>`) mainly because libstdc++'s `<vector>` pulls in
less total header code than its `<string>`, so there is less parsing to save.

## Files

- [`example.cpp`](example.cpp) — the test program
- [`measure.sh`](measure.sh) — reproduces every number above
