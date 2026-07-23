# Case study: how complete is psychicstd's `std::string`?

`std::string` is probably the single most-used class in the STL, so "is
psychicstd usable?" largely comes down to "is its `std::string` complete
enough?" This study answers that quantitatively, and along the way explains why
psychicstd's compile-speed advantage is *architectural* rather than an artifact
of being unfinished.

## Method

We count the member functions of `std::basic_string` in psychicstd and in the
system libstdc++ **from clang's JSON AST dump** — the authoritative list of
declarations the compiler actually sees, not a regex or line count. Each member
is classified by kind (constructor / destructor / method / conversion) and, via
the AST's access markers, as public or private.

Reproduce with:

```bash
./count_members.py
```

Environment: clang 21.1.8, `-std=c++20`. (The script re-runs against whatever
libstdc++ your clang picks up, so numbers may shift slightly by GCC version.)

## Results

| | psychicstd | libstdc++ |
| --- | --- | --- |
| Constructors | 11 | 12 |
| Destructor | 1 | 1 |
| Methods | 106 | 160 |
| Conversions | 1 | 1 |
| **Total members** | **119** | **174** |
| Distinct method names | 49 | 76 |
| — of which public | **45** | **45** |

By raw member count psychicstd has 68 % as many declarations. It has the same
number of distinct public method names as libstdc++, although the sets differ
by one name.

**What psychicstd is missing (32 distinct names):**

- **Public API gap — only 1:** `copy`.
- **Private implementation helpers — 31:** `_M_construct`, `_M_create`,
  `_M_dispose`, `_M_is_local`, `_M_local_data`, `_M_mutate`, `_M_replace_cold`,
  `_S_allocate`, `_S_copy`, … — the small-string-optimization and
  allocator-awareness machinery. **psychicstd doesn't need these**, because it
  deliberately has no SSO and a simpler internal design.

**What psychicstd adds:** `contains` (public), plus its own small set of private
helpers (`alloc`, `dealloc`, `grow`, `empty_str`).

## Analysis

**Usability: `std::string` is effectively complete.** It has 45 distinct
public method names, the same count as libstdc++, with `copy` missing and
C++23's `contains` present instead. For everyday development, psychicstd's
`std::string` is a drop-in.

**The size gap is private machinery, not missing features.** 31 of the 32
missing names are libstdc++'s internal `_M_*`/`_S_*` helpers. The member-count
gap and equal public-name counts tell the story in one line: psychicstd
implements nearly the same public API with far less internal apparatus. That
apparatus (SSO, allocator
propagation, the SFINAE/constraints wrapped around each overload) is exactly
what makes libstdc++'s `<string>` slow to parse and instantiate — see the
[`<string>` speed study](../std_string/stdstringcasestudy.md).

**Why this matters for the compile-speed advantage.** A natural worry is that
psychicstd is only fast *because* it is unfinished, and that the advantage will
erode to nothing as it becomes complete (especially under
[modules](../modules/modulescasestudy.md), where the gap already narrows to
~1.65×). The member breakdown argues otherwise:

- The public API is already ~complete, so there is little left to "fill in" that
  would add cost.
- The advantage comes from *how* string is built (no SSO, minimal constraints,
  few includes) — a permanent design choice — not from *how many* methods it
  has. That leanness is precisely what survives modules, because modules cache
  parsing but the translation unit still instantiates, and psychicstd's simpler
  types instantiate faster.

**The real risk is compliance pressure, not feature count.** The way to lose the
advantage is to reintroduce the machinery — full allocator propagation,
`constexpr` string, exact overload resolution via constraints, edge-case
correctness. The discipline that protects the moat is adding capability *without*
adding apparatus.

## Caveats

- Presence of a public method *name* is not full standards compliance: overload
  sets, template constraints, `noexcept`, `constexpr`, and edge-case semantics
  are not checked here (the [libcxx-based compliance suite](../../compliance.md)
  covers behavior).
- `std::string` is the most complete part of psychicstd; other headers are less
  so. This study is deliberately scoped to it because it is the highest-value
  case.

## Files

- [`count_members.py`](count_members.py) — dumps the AST for both libraries and reports the counts and the missing/extra members
