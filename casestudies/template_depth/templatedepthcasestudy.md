# Case study: template instantiation vs. raw byte count

**Hypothesis:** template instantiation dominates compile time far more than raw
source-byte count does. A small amount of template code can generate a large
amount of compiler work — so the *kind* of code matters more than how much of
it there is.

This is the complement to the [include-overhead study](../include_overhead/includeoverheadcasestudy.md),
which found that reducing *bytes* and *files* is only a second-order win. If
instantiation is the real cost driver, then reducing template machinery is the
highest-leverage optimization — and that is exactly what psychicstd does (no
small-string optimization, minimal traits), which is why the `<string>` and
`<vector>` studies saw libstdc++ burn 20–35 ms in `InstantiateClass` /
`InstantiateFunction` while psychicstd spent almost none.

## Method

We generate self-contained C++ programs with four strategies, each producing a
tunable amount of work `M`, and sweep `M`:

| Strategy | What it generates | Source | Instantiation |
| --- | --- | --- | --- |
| `plain` | `M` hand-written structs, each with a method | **lots** | ~none (baseline) |
| `instances` | 1 class template instantiated at `M` distinct args | tiny | `M` light |
| `instances_heavy` | `M` instantiations, each triggering a nested trait chain | tiny | `M` heavy |
| `recursive` | a single inheritance chain of depth `M` | ~constant | depth `M` |

The comparisons isolate one variable each:

- **`plain` vs `instances`** — same resulting entities (`M` types + `M`
  methods), but `instances` expresses them in ~1/M the source. Isolates
  *bytes vs instantiation*.
- **`instances` vs `instances_heavy`** — same `M`, ~same source, but each heavy
  instantiation forces ~13 nested instantiations. Isolates *per-instantiation
  complexity*.
- **`recursive`** — source is essentially constant (~170 bytes) while
  instantiation depth grows with `M`. Isolates *pure depth*.

Everything is self-contained (no STL), so this measures the pure effect.

Reproduce with:

```bash
./run_experiment.py
```

Environment: clang 21.1.8, `-std=c++20`, Linux x86-64, 7 reps per point
(median), warm cache.

## Results

Compile time and, crucially, **time per KB of source** for each strategy:

| M | `plain` | `instances` | `instances_heavy` | `recursive` |
| --- | --- | --- | --- | --- |
| 50 | 27.0 ms | 27.8 ms | 32.2 ms | 19.7 ms |
| 100 | 31.7 ms | 30.7 ms | 44.7 ms | 22.1 ms |
| 200 | 42.0 ms | 42.6 ms | 66.0 ms | 23.0 ms |
| 400 | 63.7 ms | 63.6 ms | 113.2 ms | 25.9 ms |
| 800 | 109.1 ms | 109.0 ms | 211.6 ms | 40.1 ms |
| 1600 | 198.5 ms | 198.1 ms | 415.3 ms | 83.9 ms |

Time per KB of source at M=1600 (fixed startup overhead amortized):

| Strategy | source at M=1600 | ms/KB |
| --- | --- | --- |
| `plain` | 144 KB | **1.4** |
| `instances` | 34 KB | **5.9** (4.2×) |
| `instances_heavy` | 36 KB | **11.8** (8.4×) |
| `recursive` | 170 **bytes** | **505** (~360×) |

## Analysis

**1. Compile cost tracks instantiations, not bytes.** `plain` and `instances`
produce the same `M` types and `M` methods. `instances` writes them in **4.2×
less source**, yet compiles in **exactly the same time** (198.1 vs 198.5 ms at
M=1600). The bytes bought nothing; the work is in materializing the entities.
Per source byte, the template version is 4.2× more expensive — not because
templates are slow, but because a byte of template source stands for far more
compiler work than a byte of plain source.

**2. Instantiation that begets instantiation multiplies the cost.**
`instances_heavy` has the same `M` and nearly the same source as `instances`,
but each instantiation drags in a ~13-deep trait chain. Result: **2.1× slower**
(415 vs 198 ms at M=1600). This is the STL cost model in miniature — one
`std::vector<T>` pulls in iterators, allocator traits, and more, each its own
instantiation.

**3. Depth is almost free in bytes and expensive in time.** `recursive` is
~170 bytes of source at *every* `M`, yet compile time climbs from 20 ms to
84 ms as depth goes 50 → 1600. At 505 ms/KB it is **~360× more expensive per
byte than plain code**: those 170 bytes cost as much as ~60 KB of plain source.

**Takeaway:** the byte count of a header is a poor predictor of its compile
cost; the amount of template instantiation it forces is a far better one. This
validates psychicstd's core strategy — the biggest compile-speed wins come from
*doing less template work* (simpler types, shallower trait chains, fewer
instantiations), not merely from shipping fewer bytes or fewer files. When
optimizing a psychicstd header, look first at what it instantiates.

## Files

- [`run_experiment.py`](run_experiment.py) — generates the programs, runs the sweep, reproduces every number above
