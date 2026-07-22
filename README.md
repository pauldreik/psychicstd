# psychicstd - a C++ standard library implementation

This is a **highly experimental** C++ standard library optimized for compilation speed. It is intended to be used during general C++ development to speed up the edit-compile-debug cycle. It is not at all intended to be used for shipping binaries.

It is not complete. It is not fully compliant. But it is good enough to quickly iterate on code. Here are some real world projects that compile and pass their unit tests with psychicstd. The number in the second column indicates the speedup relative to the platform standard library for the compilation phase (1x means same speed, higher is better):

| Project | Compile time speedup | comment |
|-------------------------------------------------------------|-----------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------|
| [Boost.Asio](https://www.boost.org/libs/asio/) | [2.15x](use_on_realworld_projects/boost-asio_speed_report.md) | |
| [catch2](https://github.com/catchorg/Catch2) | [3.63x](use_on_realworld_projects/catch2_speed_report.md) | |
| [cmake](https://cmake.org/) | [3.26x](use_on_realworld_projects/cmake_speed_report.md) | Uses a compiler wrapper to build. |
| [cppcheck](https://github.com/cppcheck-opensource/cppcheck) | [2.07x](use_on_realworld_projects/cppcheck_speed_report.md)| |
| [eigen](https://gitlab.com/libeigen/eigen) | [1.90x](use_on_realworld_projects/eigen_speed_report.md) | |
| [fmt](https://github.com/fmtlib/fmt) | [1.67x](use_on_realworld_projects/fmt_speed_report.md) | |
| [googletest](https://github.com/google/googletest) | [1.58x](use_on_realworld_projects/googletest_speed_report.md) | |
| [nlohmann json](https://json.nlohmann.me/) | [2.05x](use_on_realworld_projects/nlohmann_speed_report.md) | Uncovered a reliance on implementation-specific behaviour, fixed in [PR #5236](https://github.com/nlohmann/json/pull/5236). |
| [OpenCV](https://opencv.org/) | [1.83x](use_on_realworld_projects/opencv_speed_report.md) | Builds the core and imgproc modules and their tests. |
| [rapidjson](https://github.com/Tencent/rapidjson/) | [1.26x](use_on_realworld_projects/rapidjson_speed_report.md) | Not using much of the standard library, little speedup expected. |
| [rdfind](https://rdfind.pauldreik.se/) | [4.16x](use_on_realworld_projects/rdfind_speed_report.md) | Runs in psychic strict mode, see "Compatibility levels" further down this document. Strict mode uncovered code relying on transitive includes. |
| [simdutf](https://github.com/simdutf/simdutf) | [1.64x](use_on_realworld_projects/simdutf_speed_report.md) | Mostly SIMD intrinsics. [Strict mode uncovered missing includes](https://github.com/simdutf/simdutf/pull/998); the pinned test suite currently runs in drop-in mode because a helper omits `<cstdlib>`. |
| [wordcounter](benchmarks/compile_time/bench_wordcounter.cpp)| [4.0x](speed.md) | [demo program using STL](benchmarks/compile_time/bench_wordcounter.cpp). Counts word occurrences in text files. |

Find the scripts validating the build and generating the above number in the [use_on_realworld_projects/](use_on_realworld_projects) directory. Platform-wide measurements are available for [Linux](speed.md) and [macOS](speed_macos.md), including separate process-startup results for [Linux](startup.md) and [macOS](startup_macos.md).

Static linking also avoids the `libstdc++.so.6` and `libm.so.6` dependencies. A representative Linux program measured [1.70x faster exec-to-exit](startup.md), including loading, initialization, and its small fixed workload.

Once you have coded for a while, switch to a real quality standard library (typically libstdc++ or libc++) and test and build real releases - psychicstd is just intended for speeding up the development.

## How complete is it?

Completeness varies by header, development has been guided what is needed to get realworld
projects to compile. The `std::string` header is perhaps the most used header and to
investigate the complementess I counted the number of public member functios to see what
is missing. It has **45 distinct public `basic_string` method names**, the same count as libstdc++: psychicstd is missing `copy` but adds C++23's `contains`. See the [`std::string` completeness case study](casestudies/string_completeness/stringcompletenesscasestudy.md) for details. Note that a present method name is not full compliance — see [compliance.md](compliance.md) for behavioral coverage.

Some facilities are omitted deliberately when their practical value does not
justify the compile-time cost. For example, `<iostream>` provides the commonly
used narrow streams, but not `wcin`, `wcout`, `wcerr`, or `wclog`. Defining
those rarely used wide streams made a representative program about 6–7% slower
to compile even when it did not use them. Such gaps are preferred over making
every user pay for unused compliance.

## Why?

Slow compilation is one of the pain points of C++. Modules are supposed to help, but it is not yet ready. Also, even if modules solve the slow parsing of include files problem, standard libraries are typically optimized for runtime performance. Psychichstd does not care about runtime performance - it is all about compilation speed.

I got the idea when I read about the [pystd](https://github.com/jpakkane/pystd) project from Jussi Pakkanen, which has completely different goals but got me thinking.

Writing a standard library is a massive undertaking and super hard with lots of corner cases. Three factors in combination makes it possible anyway:

- AI assisted coding
- the excellent libc++ test suite
- no demands on portability, runtime performance, ABI stability etc

An AI can mostly guide itself trying to get the library through all the tests. Anything the AI can generate that passes the test suite is useful.

## How does it work?

Psychicstd uses `-nostdinc++ -I/path/to/psychicstd/include` so the compiler
picks up vector, string and other headers from psychicstd instead of
libstdc++. A small static library supplies code deliberately moved out of the
headers: standard stream objects, cold exception and system-error paths,
string conversions, and selected narrow-character template instantiations.
Consumers must link this library. The CMake toolchain overlay builds and links
it automatically; manual integrations must build and link it explicitly.

The standard library uses `std::` namespace just like the real standard. You should not have to do any changes to your program.

## Why is it faster?

Compile time for these headers is dominated by the compiler *frontend*, in two parts: **parsing declarations** and **instantiating templates**. psychicstd wins by having far less of both. Raw byte count, number of include files, and backend code generation are all second-order. Precompiled headers help by caching that same frontend work — but they attack the same bottleneck, so psychicstd without a PCH is roughly as fast as libstdc++ with one, and still wins when both use PCH. The [case studies](casestudies/) measure each of these effects.

A concrete example is that `std::sort` has a very short and simple implementation to minimize compile time. It is still O(Nlog(N)) but not as fast as other standard libraries **in release mode**. In debug mode, it can however even be faster!

The [compiled-library experiment](docs/compiled-library.md) measures the
compile-time and linked-size effects of outlining these paths, explicitly
instantiating narrow strings and stringstreams, and splitting the runtime into
independently extracted archive members.

## Compatibility levels

Real standard library headers pull in a lot of other headers transitively. A lot of real code accidentally relies on that — e.g. using `std::equal` after only `#include <string>`, which happens to work because libstdc++'s `<string>` drags in `<algorithm>`. That is technically a bug (the code should `#include <algorithm>`), but it is extremely common.

psychicstd lets you choose how to handle it, via the `_PSYCHICSTD_COMPATIBILITY_LEVEL` macro:

| Level | Macro value | Behaviour |
|---|---|---|
| **Drop-in** (default) | `_PSYCHICSTD_COMPAT_DROPIN` (`2`) | Mirror libstdc++'s transitive include surface, so unmodified real-world code just compiles. No source changes needed. |
| **Strict** | `_PSYCHICSTD_COMPAT_STRICT` (`0`) | Each header includes only what it itself needs. Fastest and smallest — and it doubles as an *include-what-you-use checker*: code that leaned on a transitive include fails to compile until you add the missing `#include` (a fix that is also correct under libstdc++/libc++). |

Set it like any other define, alongside the other psychicstd flags:

```bash
cmake -S . -B build \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -isystem /path/to/psychicstd/include -D_PSYCHICSTD_COMPATIBILITY_LEVEL=0"
```

The default is drop-in, so you get the "just swap it in" experience out of the box; opt into strict when you want maximum speed and to keep your includes honest.

The strict mode results in faster compilation since fewer headers are transitively included. For rdfind, strict mode shrank the compilation time with 5% compared to the drop-in level.

## The name

The name is a word play on the edit-compile-debug cycle itself: psychic → cycle → cyclic. A psychic knows the answer before you've finished asking the question. psychicstd tries to do the same — by the time you've hit save, the compiler is already done. Well, that's the aspiration, anyway.

## Goals

These are the goals, in order

- **Useful in practice** — code should compile, link, and run well enough to support a normal development workflow.
- **Free of UB** — projects that are UB free, shall be UB free also when using psychic.
- **Faster compilation** — If it is not faster to compile than real implementations like libstdc++, this project has very little value.
- **Sufficiently compliant to the C++ standard** — it should be correct enough to be useful. For example, `std::string` does not need to use small string optimization, which simplifies the implementation.
- **Support C++20**

## Non-goals

- Runtime performance — we don't try to make your program run faster, only to compile faster. With that said, typical unit tests are expected to run decently fast. On the example projects, psychicstd is sometimes **faster** on tests!
- Compilation speed in release mode
- Portability beyond Linux and macOS. GCC and Clang are supported on Linux; AppleClang is supported on macOS 14.4 and newer. There is no Windows or MSVC support.
- ABI stability or any kind of guarantees
- Support older C++ standards

## Development guidelines

All code and text should be auto formatted. Use the following:

- `run_clang_format.sh` (uses the .clang-format in the git root)
- `run_cmake_format.sh`
- `run_markdown_format.sh`
- `run_python_lint.sh` (formats and checks with ruff)
- `run_shell_format.sh`
- `run_yaml_format.sh` (formats with yamlfix)

Unit tests should pass. The compliance test need not, getting 100% compliance is utopic.

## Use in your project

No changes to your source code are needed. You inject compile and link flags
at configure time; your project's own build files stay untouched.

The CMake toolchain described below is the recommended integration and builds
the runtime as part of the consuming project. If you inject flags manually,
first configure a separate psychicstd build with the same compiler as the
consumer. Replace `/path/to/your-c++` with that compiler, for example `g++-14`
or `clang++`:

```bash
cmake -S /path/to/psychicstd -B /path/to/psychicstd/build-runtime \
    -DCMAKE_CXX_COMPILER=/path/to/your-c++ \
    -DPSYCHICSTD_BUILD_TESTS=OFF \
    -DPSYCHICSTD_BUILD_BENCHMARKS=OFF
cmake --build /path/to/psychicstd/build-runtime --target psychicstd
```

With a single-configuration Makefiles or Ninja generator, the manual examples
below call that output
`/path/to/psychicstd/build-runtime/libpsychicstd.a`. Rebuild it when changing
compiler or instrumentation flags that should also apply to the runtime.

### CMake / GCC 12

GCC 12 lacks `-nostdlib++`, so you need `-nodefaultlibs` and all the
libraries spelled out:

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_COMPILER_WORKS=1 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -fvisibility=hidden -isystem /path/to/psychicstd/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="/path/to/psychicstd/build-runtime/libpsychicstd.a -lsupc++ -latomic -lm -lc -lgcc_s -lgcc"
```

`CMAKE_CXX_COMPILER_WORKS=1` skips the compiler detection link test (the test
program needs libc, but `CMAKE_CXX_STANDARD_LIBRARIES` isn't applied during
detection).

### CMake / GCC 13+

GCC 13 added `-nostdlib++`, which drops libstdc++ while keeping libc, libm,
libgcc_s and libgcc. Add `libsupc++` for the C++ ABI and `libatomic` for
generic atomic operations that the compiler cannot lower inline:

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -fvisibility=hidden -isystem /path/to/psychicstd/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-nostdlib++" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="/path/to/psychicstd/build-runtime/libpsychicstd.a -lsupc++ -latomic"
```

No `CMAKE_CXX_COMPILER_WORKS` needed — libc is still linked by default.

### CMake / Clang

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -fvisibility=hidden -isystem /path/to/psychicstd/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-nostdlib++" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="/path/to/psychicstd/build-runtime/libpsychicstd.a -lsupc++ -latomic"
```

### CMake toolchain file

For CMake projects, the most convenient integration point is the toolchain
overlay in [`cmake/psychicstd-toolchain.cmake`](cmake/psychicstd-toolchain.cmake).
It keeps the compiler choice outside the toolchain and only injects the
psychicstd-specific flags.

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_COMPILER=g++-14 \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/psychicstd/cmake/psychicstd-toolchain.cmake
```

The toolchain is designed to compose with user flags and sanitizer settings.
It adds an internal static-library target to the consuming build and links it
to targets declared after the top-level `project()` call, so no separate
psychicstd build step or archive path is needed.
If you already have a generated toolchain file, include the psychicstd one
after it from a small wrapper toolchain file.

### Using with Conan

If you already use Conan, the intended integration point is a single overlay
profile: [`tests/conan_project/psychic.profile`](tests/conan_project/psychic.profile).
It composes with your existing host/build profiles and injects the psychicstd
toolchain without selecting a compiler or adding dependency-specific flags.

Apply it to both host and build contexts so your app and its C++ dependencies
are built with the same standard library choice:

```bash
conan install . \
    -pr:h=your-host.profile \
    -pr:h=/path/to/psychic.profile \
    -pr:b=your-build.profile \
    -pr:b=/path/to/psychic.profile \
    --build=missing
```

If your host and build profiles are the same, reuse the same base profile for
both contexts and append the psychic profile last. File-based composition works
too:

```text
include(/path/to/psychic.profile)
```

The example in `tests/conan_project/` uses `fmt` to show a real third-party
dependency built this way. The profile does not overwrite sanitizer flags, so
ASan and UBSan keep working the way Conan or your project already configures
them. Supported compilers are the same as the toolchain-overlay path: Clang
and GCC 13+ on Linux.

### Notes for all configurations

`-fvisibility=hidden` prevents psychicstd's symbols from interposing with
libstdc++ at runtime.
`-isystem` (rather than `-I`) suppresses warnings from psychicstd headers.

To switch back to the system STL, configure without these flags.
See `tests/external_project/run.sh` for a self-contained working example.

### Autoconf/make

Pass flags as `./configure` arguments so they are baked into the Makefile —
plain `make` then works without extra flags:

```bash
./configure \
    CXXFLAGS="-std=c++20 -nostdinc++ -isystem /path/to/psychicstd/include" \
    LDFLAGS="-nodefaultlibs" \
    LIBS="/path/to/psychicstd/build-runtime/libpsychicstd.a -lsupc++ -latomic -lm -lc -lgcc_s -lgcc"
make
```

`-nodefaultlibs` drops the default libstdc++; the explicit `LIBS` supply
the necessary C++ runtime support (exceptions, operator new/delete, etc.).
GCC's own library search path is unaffected so `-lsupc++` is found without
a full path.

## Building and testing psychicstd itself

### Build and run tests

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The default build covers the library itself — no third-party code, no network access:

- each implemented header compiled against both the system STL and psychicstd, to check behavioral equivalence
- a self-containment test per header: including just that one header (and nothing else) must compile
- a simulated external project that uses the psychicstd toolchain overlay, including ASan and UBSan variants

### Testing on real-world projects

Correctness in practice is verified by compiling — and running the test suites of — actual third-party projects against psychicstd. The scripts in [`use_on_realworld_projects/`](use_on_realworld_projects/) clone, build, and run Boost.Asio, Catch2, cppcheck, Eigen, fmt, GoogleTest, nlohmann JSON, OpenCV, RapidJSON, rdfind, and simdutf. The CMake recipe builds its supported upstream KWSys and utility targets. These recipes produce the speedup reports linked at the top of this README.

### Benchmarks

Since this library is all about compilation speed, there is benchmarking to measure compilation speed.

```bash
cmake --build build --target bench
# or directly:
python3 benchmarks/compile_time/run_bench.py
```

View the results by inspecting [speed.md](speed.md).

There is also a benchmark for process startup time (exec-to-exit time, not an isolated dynamic-linker measurement — see the note above):

```bash
cmake --build build --target startup_bench
# or directly:
python3 benchmarks/startup_time/run_bench.py <path-to-system-binary> <path-to-psychicstd-binary>
```

View the results by inspecting [startup.md](startup.md).

For a deeper look at *why* psychicstd compiles faster, see the per-header case studies, which use clang's `-ftime-trace` to break down where the time goes:

- [`<string>` compilation speed case study](casestudies/std_string/stdstringcasestudy.md)
- [`<vector>` compilation speed case study](casestudies/std_vector/stdvectorcasestudy.md)
- [does the *number* of include files matter?](casestudies/include_overhead/includeoverheadcasestudy.md) — testing whether fewer, larger headers compile faster
- [template instantiation vs. raw byte count](casestudies/template_depth/templatedepthcasestudy.md) — showing instantiation work, not bytes, drives compile time
- [precompiled headers vs. psychicstd](casestudies/precompiled_headers/precompiledheaderscasestudy.md) — how PCH stacks with psychicstd, and whether psychicstd still wins
- [C++20 modules vs. psychicstd](casestudies/modules/modulescasestudy.md) — header units vs. PCH, and why modules close the gap the most

## Standards compliance

Psychicstd uses the libc++ unit tests to partially ensure the library is standards compliant. You need to check out the source code separately from [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project).

Running these tests takes a very long time - they are extensive! For that reason, by default only a random subset is used for each header.

Note that those tests are sometimes libc++ specific - there is not necessarily anything wrong just because a particular test does not pass.

Psychicstd prioritizes getting real projects running (that is: compiling and passing their unit tests) rather than maximizing the score on the libc++ tests.

```bash
python3 tools/compliance.py
```

It samples up to 15 libcxx test files per header, runs them against both the system STL (as a baseline filter) and psychicstd, and updates `compliance.md` with per-header results. Each header gets two indicators:

- **Compliance** — what fraction of the sampled libcxx tests pass: 🟢 ≥80%, 🟡 compiles but fewer pass, 🔴 does not compile with psychicstd.
- **Speed** — median compile time of the libcxx test files, psychicstd vs system: 🟢 >1.2×, 🟡 similar, 🔴 slower.

See the results in [compliance.md](compliance.md).

The same suite can be run under AddressSanitizer and UndefinedBehaviorSanitizer
(`python3 tools/compliance.py --sanitize`) to catch memory bugs, UB, and
behavioral divergence, gated against a baseline of known failures. See
[docs/sanitizer-testing.md](docs/sanitizer-testing.md).
