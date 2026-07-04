# Case study: does the *number* of include files matter?

**Hypothesis:** for a fixed total amount of code, splitting it across more
`#include`d files makes compilation slower — each extra file costs something
(path lookup, `stat`/`open`/`read`, lexing, include-guard handling) beyond the
code it contains.

This is a lever we could pull on psychicstd: if fewer, larger headers compile
faster than many small ones, consolidating headers would be an easy win.

## Method

We isolate *file count* from *amount of code*:

1. Generate one fixed body of C++ — the **same sequence of code units** every
   run (each unit is a small self-contained `struct` with a couple of methods).
1. Partition that fixed body into `K` header files for several values of `K`
   (1, 2, 5, … 2000). A generated `main.cpp` `#include`s all `K`.
1. Because the sequence of units is identical regardless of `K`, the parser
   sees the same declarations every time — **only the number of files changes.**
1. Measure median compile time vs `K`. Confirm the parser-visible code is truly
   constant by stripping line-marker directives from the preprocessed output and
   checking the byte count doesn't move. A linear fit gives the per-file cost.

The generated code has **no STL includes**, so this measures the pure
file-count effect, independent of which standard library is in use.

Reproduce with:

```bash
./run_experiment.py
```

Environment: clang 21.1.8, `-std=c++20`, Linux x86-64, 8000 code units
(~975 KB of parser-visible code), 7 reps per `K` (median), warm cache.

## Results

| K (files) | parser-visible code | median compile time |
| --- | --- | --- |
| 1 | 974,914 B | 389.3 ms |
| 2 | 974,914 B | 389.4 ms |
| 5 | 974,914 B | 390.4 ms |
| 10 | 974,914 B | 390.5 ms |
| 25 | 974,914 B | 388.1 ms |
| 50 | 974,914 B | 393.1 ms |
| 100 | 974,914 B | 391.2 ms |
| 250 | 974,914 B | 401.1 ms |
| 500 | 974,914 B | 416.9 ms |
| 1000 | 974,914 B | 440.8 ms |
| 2000 | 974,914 B | 484.7 ms |

The parser-visible code is byte-for-byte identical across every row (0.00 %
spread) — only the file count varies.

- Linear fit: `time ≈ 389.4 ms + 48.6 µs/file × K`
- At K=2000 the *same code* compiles **1.25× slower** than at K=1.
- **Per-extra-file overhead ≈ 49 µs.**

## Analysis

**The hypothesis is true** — more files cost more, for identical code, at
roughly a constant ~49 µs per file. So consolidating headers is a genuine
lever.

**But the magnitude is small at realistic file counts.** The overhead only
becomes visible in the hundreds of files: at K=100 it is ~2 ms on top of a
~390 ms baseline (\<1 %). Real standard-library headers pull in far fewer files
than that. Applying the ~49 µs/file figure to the counts measured in the other
case studies:

| Header | files (system → psychicstd) | file-count savings |
| --- | --- | --- |
| [`<string>`](../std_string/stdstringcasestudy.md) | 61 → 23 | ~1.9 ms |
| [`<vector>`](../std_vector/stdvectorcasestudy.md) | 45 → 28 | ~0.8 ms |

Those savings are ~1–2 ms out of the ~100–140 ms that psychicstd actually saves
on those headers. In other words, **file-count reduction explains only a tiny
slice of psychicstd's speedup.** The dominant factors, as the `<string>` and
`<vector>` studies show, are *less total code to parse* and *far fewer template
instantiations* — not the number of files.

**Takeaway:** fewer includes do help, and it is essentially free to consolidate,
but it is a second-order optimization. Effort aimed at compile speed is far
better spent shrinking the amount of code and template machinery each header
drags in than on merging files. Worth revisiting only for headers that fan out
into dozens-to-hundreds of small includes.

## Files

- [`run_experiment.py`](run_experiment.py) — generates the code, runs the sweep, reproduces every number above
