# Implementation techniques

psychicstd uses the following principles, roughly from most important to least
important.

## Minimize parsed and instantiated code

Compiler frontend work is dominated by the amount of code parsed and the
templates instantiated, not by the number of `#include` directives. Keep
implementations small, avoid general machinery when a narrow implementation is
sufficient, and omit declarations unrelated to a header's public surface.

**Example:** `include/algorithm` implements `std::sort` as a short heapsort
instead of carrying the larger collection of policies and specialized paths
used by a production runtime library.

## Require C++20

Maintain one modern implementation path. Concepts and `requires` expressions
replace legacy SFINAE overload sets, while `if constexpr` replaces tag dispatch
and helper specializations. There are no compatibility implementations for
older language modes.

**Example:** the converting constructors in `include/optional` state their
constraints directly with `requires` instead of adding `enable_if` parameters
and duplicate overloads.

## Do not preserve an internal ABI

psychicstd does not promise ABI stability and therefore has no versioned inline
namespace, compatibility layouts, old symbol aliases, or frozen implementation
details. Public types are declared directly in `namespace std`, and their
representations may follow the simplest current implementation.

This also keeps mangled names, debug information, diagnostics, and debugger
type names smaller.

**Example:** `std::vector` in `include/vector` directly stores `data_`, `size_`,
`cap_`, and `alloc_`; it is not wrapped in an ABI-versioned base hierarchy.

## Split public headers into narrow internal layers

Public standard headers expose complete standard interfaces. Internal code
includes dependency-ordered implementation layers containing only the needed
parts instead of repeatedly parsing public umbrella headers.

A translation unit that includes several public headers pays for a shared
internal layer only once. Strict headers include only their real dependencies;
drop-in headers add narrow compatibility includes where projects rely on
libstdc++'s transitive surface.

**Example:** `<utility>` is split into `__psychicstd_utility_core`,
`__psychicstd_utility_operations`, and `__psychicstd_utility_pair`. `<optional>`
uses the first two without parsing `pair`.

## Prefer direct language operations and compiler builtins

When a standard facility maps directly to a language operation or a supported
compiler builtin, use that representation instead of a general template
framework.

**Example:** `include/__psychicstd_type_traits_object` implements traits such as
`is_constructible` and `is_assignable` with `__is_constructible` and
`__is_assignable`. `include/bit` implements `bit_cast` with
`__builtin_bit_cast`.

## Exploit the small compiler and platform matrix

Supporting current GCC and Clang on Linux and AppleClang on macOS allows the
implementation to use known builtins and platform behavior directly. There is
no abstraction layer for MSVC or old compiler frontends.

When GCC and Clang expose different forms, keep only the small branches needed
by those supported compilers.

**Example:** `make_integer_sequence` in `include/__psychicstd_utility_core`
selects Clang's `__make_integer_seq` or GCC's `__integer_pack` directly.

## Keep representations and common paths simple

Prefer pointers, sizes, direct ownership, short loops, and ordinary branches
over policy classes, dispatch layers, and elaborate storage frameworks.
Runtime optimizations that substantially complicate headers are omitted when
they do not serve the edit-compile-debug use case.

**Examples:**

- `vector` in `include/vector` stores one pointer plus a size and capacity,
  rather than begin, end, and end-cap pointers. Size and capacity arithmetic is
  direct, and allocator pointer handling stays localized.
- `basic_string` in `include/string` always stores a pointer, size, capacity,
  and allocator. It has no small-string optimization, avoiding tagged or union
  storage, encoded capacity bits, and short-versus-long branches in
  construction, assignment, growth, movement, and destruction.

## Move stable, cold work out of headers and split the archive

Non-template cold paths, global state, and selected explicit instantiations
live in the runtime library. This avoids parsing and instantiating the same work
in every consumer translation unit and reduces consumer Debug object code.

Tiny hot operations remain inline. Unrelated runtime facilities are compiled
into separate object files so the static linker extracts only the pieces a
program references. Do not combine source files merely because their public
facilities share a header.

**Example:** `cin`, `cout`, `cerr`, and `clog` have separate source files, so a
program using one stream does not acquire every stream object. Cold exception,
error, and conversion paths are similarly separated into `src/stdexcept.cpp`,
`src/system_error.cpp`, and `src/string.cpp`. The archive membership is listed
in `cmake/psychicstd-runtime-sources.txt`.

## Specialize the dominant concrete case

Keep generic public interfaces, but compile common concrete instantiations once
when that removes repeated template work from consumers. Less common cases can
retain their header implementation.

**Example:** `src/string_instantiations.cpp` explicitly instantiates
`basic_string<char>`, while other character types remain generic.

## Use `if constexpr` to discard irrelevant machinery

When a type property selects fundamentally different implementations,
`if constexpr` discards the unused branch before instantiation. Separate
overloads remain preferable when combining them would create one large function
containing unrelated behavior.

**Example:** `include/vector` selects the ordinary allocator path and its fancy
pointer fallback with `if constexpr`, so each instantiation contains only the
applicable allocation operations.

## Keep trivial helpers out of debug stacks

Cast-only vocabulary functions should not become debugger frames in
unoptimized builds. Inside the implementation, use the exact cast directly:

```cpp
// Equivalent to std::move(value).
static_cast<remove_reference_t<T>&&>(value)

// Equivalent to std::forward<T>(value).
static_cast<T&&>(value)
```

The `remove_reference_t` in the move form preserves its semantics when `T` is a
reference. The public functions retain their standard interfaces and use
`[[gnu::always_inline]]`.

**Example:** `include/__psychicstd_utility_operations` uses direct casts inside
`swap` and `exchange`, while its public `move` and `forward` functions are
always inline.
