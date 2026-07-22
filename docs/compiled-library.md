# Compiled library experiment

Psychicstd now has a small compiled component. The public CMake target is a
static library, while the toolchain overlay adds an equivalent private static
library to an otherwise unmodified consuming project.

The first compiled translation unit contains:

- `cin`, `cout`, `cerr`, and `clog`, including their stdio stream buffers;
- the cold `ios_base::failure` construction and throw path;
- explicit instantiations of `basic_istream<char>` and
  `basic_ostream<char>`.

The explicit instantiations are used only when exceptions are enabled.
`-fno-exceptions` consumers keep local instantiations so the header's abort
path does not depend on how the static library was compiled.

This is deliberately still one source file. It keeps the initial build and
integration simple. If linked binary size or incremental rebuilding of the
library becomes important, the next split should be `ios.cpp`, `istream.cpp`,
`ostream.cpp`, and one source file per global stream. Static archive extraction
would then avoid bringing unused input or output code into a program.

## Measurements

The probes were compiled as C++20 with Clang 22.1.8, `-O0`, `-nostdinc++`, and
`-ftime-trace-granularity=0`. Times are the median `Total ExecuteCompiler` from
15 alternating baseline and current compilations with a warm filesystem cache.
The stream-using probe is `benchmarks/compile_time/bench_iostream.cpp`.

| Probe | Before | Compiled library | Change |
| --- | ---: | ---: | ---: |
| Include `<iostream>`, empty `main` | 69.9 ms | 61.0 ms | -12.8% |
| Write an integer to `cout`, inspect `cin` | 80.4 ms | 62.8 ms | -21.9% |

For the include-only probe, generated functions fell from 72 to 14 and its
object file from 7,963 to 980 bytes. For the probe using the streams, the object
file fell from 17,089 to 1,173 bytes.

The main tradeoff is linked code size. Explicitly instantiating the complete
narrow stream classes increased the linked probe's text from 104,808 to
130,181 bytes. This is acceptable for the current edit-compile-debug goal, but
it should be remeasured on real projects. If the cost is too high, retain the
out-of-line globals and failure path but remove the explicit instantiations;
the initial trace showed that intermediate design retained most of the
compile-time improvement.

## Next candidates

Use traces from real builds before moving more code. Good candidates are
non-template cold paths which already carry `noinline`, repeated template
instantiations with stable narrow-character specializations, and global state
which should have one process-wide identity. Avoid outlining tiny hot methods:
it saves little frontend work and adds calls and ABI surface.
