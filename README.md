# psychicstd - a C++ standard library implementation

This is an **experimental** C++ standard library optimized for compilation speed. It is intended to be used during general C++ development to speed up the edit-compile-debug cycle. It is not at all intended to be used for shipping binaries.

It is not complete. It is not fully compliant. But it is good enough to quickly iterate on code. Here are some real world projects that compile and pass their unit tests with psychicstd. The number in the second column indicates the speedup relative to the platform standard library for the compilation phase (1x means same speed, higher is better):

<!-- README_BENCHMARK:realworld-speedups:start -->

| Project | Compile time speedup | comment |
|-------------------------------------------------------------|-----------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------|
| [Abseil](https://abseil.io/) | [2.13x](use_on_realworld_projects/abseil_speed_report.md) | Builds `absl/base` and eight small upstream base tests. |
| [Bitcoin Core](https://github.com/bitcoin/bitcoin) | [2.14x](use_on_realworld_projects/bitcoin_speed_report.md) | Builds all 16 crypto primitives plus memory cleansing, including MuHash and SipHash. |
| [Boost.Asio](https://www.boost.org/libs/asio/) | [2.08x](use_on_realworld_projects/boost-asio_speed_report.md) | |
| [Boost.Test](https://www.boost.org/libs/test/) | [1.56x](use_on_realworld_projects/boost-test_speed_report.md) | Builds and runs three upstream header-only smoke tests; the nightly `boost-test-full` variant runs the complete upstream Boost.Build target. |
| [catch2](https://github.com/catchorg/Catch2) | [3.44x](use_on_realworld_projects/catch2_speed_report.md) | |
| [cmake](https://cmake.org/) | [3.00x](use_on_realworld_projects/cmake_speed_report.md) | Uses a compiler wrapper to build. |
| [cppcheck](https://github.com/cppcheck-opensource/cppcheck) | [1.99x](use_on_realworld_projects/cppcheck_speed_report.md)| |
| [CTRE](https://github.com/hanickadot/compile-time-regular-expressions) | [1.12x](use_on_realworld_projects/ctre_speed_report.md) | Builds every upstream compile-time test. |
| [date](https://github.com/HowardHinnant/date) | [2.30x](use_on_realworld_projects/date_speed_report.md) | Builds the time-zone library and 82 portable upstream tests in date's C-locale mode. |
| [eigen](https://gitlab.com/libeigen/eigen) | [1.81x](use_on_realworld_projects/eigen_speed_report.md) | |
| [Electron](https://www.electronjs.org/) | [3.51x](use_on_realworld_projects/electron_speed_report.md) | Builds and tests a focused slice of Electron's startup and command-line handling without a Chromium checkout. |
| [FlatBuffers](https://flatbuffers.dev/) | [2.33x](use_on_realworld_projects/flatbuffers_speed_report.md) | Builds the compiler, library, samples, and C++ test suite. |
| [fmt](https://github.com/fmtlib/fmt) | [1.59x](use_on_realworld_projects/fmt_speed_report.md) | |
| [Godot Engine](https://godotengine.org/) | [1.08x](use_on_realworld_projects/godot_speed_report.md) | Builds the core static library with SCons; rendering, window-system, audio, and optional engine modules are disabled. |
| [googletest](https://github.com/google/googletest) | [1.56x](use_on_realworld_projects/googletest_speed_report.md) | Builds GoogleTest's upstream tests; the nightly `googletest-full` variant also builds GoogleMock, samples, and the complete configured test suite. |
| [HarfBuzz](https://harfbuzz.github.io/) | [1.30x](use_on_realworld_projects/harfbuzz_speed_report.md) | Builds HarfBuzz's static libraries with Meson and runs its dependency-free upstream tests; optional integrations and command-line tools are disabled. |
| [libcamera](https://libcamera.org/) | [4.28x](use_on_realworld_projects/libcamera_speed_report.md) | Builds the core libraries and UVC pipeline handler; hardware-dependent applications, optional integrations, bindings, and tests are disabled. |
| [libtorrent](https://github.com/arvidn/libtorrent) | [1.92x](use_on_realworld_projects/libtorrent_speed_report.md) | Builds the complete static library with DHT, encryption, extensions, I2P, logging, and streaming enabled; WebTorrent is disabled because its bundled dependencies are absent from the release archive. |
| [llama.cpp](https://github.com/ggml-org/llama.cpp) | [1.52x](use_on_realworld_projects/llama.cpp_speed_report.md) | Builds `ggml-base` and `ggml-cpu` and compiles model-architecture and hyperparameter handling; excludes accelerator backends, tools, examples, server, and tests. |
| [nlohmann json](https://json.nlohmann.me/) | [2.00x](use_on_realworld_projects/nlohmann_speed_report.md) | Uncovered a reliance on implementation-specific behaviour, fixed in [PR #5236](https://github.com/nlohmann/json/pull/5236). |
| [cxxopts](https://github.com/jarro2783/cxxopts) | [4.61x](use_on_realworld_projects/cxxopts_speed_report.md) | Compiles the upstream example with its default regex parser; the nightly `cxxopts-full` variant runs the complete tests and CMake integration checks. |
| [inipp](https://github.com/mcmtroffaes/inipp) | [3.13x](use_on_realworld_projects/inipp_speed_report.md) | Builds the header smoke test and runs the complete narrow-character upstream unit suite. |
| [OpenCV](https://opencv.org/) | [1.80x](use_on_realworld_projects/opencv_speed_report.md) | Builds the core and imgproc modules and their tests. |
| [pocketfft](https://github.com/mreineck/pocketfft) | [1.53x](use_on_realworld_projects/pocketfft_speed_report.md) | Compiles and runs the upstream demonstration. |
| [pybind11](https://github.com/pybind/pybind11) | [2.34x](use_on_realworld_projects/pybind11_speed_report.md) | Builds pybind11's upstream CMake extension test, imports the module in Python, and calls its bound C++ function. |
| [rapidjson](https://github.com/Tencent/rapidjson/) | [1.25x](use_on_realworld_projects/rapidjson_speed_report.md) | Not using much of the standard library, little speedup expected. |
| [rdfind](https://rdfind.pauldreik.se/) | [4.10x](use_on_realworld_projects/rdfind_speed_report.md) | Runs in psychic strict mode, see "Compatibility levels" further down this document. Strict mode uncovered code relying on transitive includes. |
| [simdutf](https://github.com/simdutf/simdutf) | [1.71x](use_on_realworld_projects/simdutf_speed_report.md) | Mostly SIMD intrinsics. [Strict mode uncovered missing includes](https://github.com/simdutf/simdutf/pull/998). |
| [SSVStart](https://github.com/vittorioromeo/SSVStart) | [3.68x](use_on_realworld_projects/ssvstart_speed_report.md) | Compiles the upstream input-utility test against SSVStart's current header dependencies. |
| [strong_type](https://github.com/rollbear/strong_type) | [3.54x](use_on_realworld_projects/strong-type_speed_report.md) | Builds and runs the complete upstream self-test suite. |
| [Tesseract](https://github.com/tesseract-ocr/tesseract) | [2.13x](use_on_realworld_projects/tesseract_speed_report.md) | Builds the OCR library and command-line program using the system Leptonica package. |
| [TensorFlow](https://www.tensorflow.org/) | [2.30x](use_on_realworld_projects/tensorflow_speed_report.md) | Builds a focused set of core platform/base libraries; excludes Python, CUDA, kernels, and the full framework. |
| [Trompeloeil](https://github.com/rollbear/trompeloeil) | [1.60x](use_on_realworld_projects/trompeloeil_speed_report.md) | Builds the complete self-test suite and runs the coroutine, threaded, and custom-mutex tests. |
| [Zancle](https://github.com/vittorioromeo/zancle) | [2.27x](use_on_realworld_projects/zancle_speed_report.md) | Builds the complete Base/System static library. |
| [wordcounter](examples/wordcounter.cpp)| [3.63x](compile_time.md) | Example application using the STL. Counts word occurrences in text files. |

Across the listed workloads, the geometric-mean compile-time speedup is **2.18x** and the median speedup is **2.13x**.

<!-- README_BENCHMARK:realworld-speedups:end -->

The real-world project recipes and reports live in [use_on_realworld_projects/](use_on_realworld_projects). Focused compile-time and compiler-memory measurements are in [compile_time.md](compile_time.md), with separate process-startup results for [Linux](startup.md) and [macOS](startup_macos.md).

<!-- README_BENCHMARK:startup-speedup:start -->

Static linking also avoids the `libstdc++.so.6` and `libm.so.6` dependencies. A representative Linux program measured [1.69x faster exec-to-exit](startup.md), including loading, initialization, and its small fixed workload.

<!-- README_BENCHMARK:startup-speedup:end -->

Once you have coded for a while, switch to a real quality standard library (typically libstdc++ or libc++) and test and build real releases - psychicstd is just intended for speeding up the development.

## How complete is it?

Completeness varies by header; development has been guided by what real-world
projects need to compile. To investigate `std::string`, perhaps the most-used
header, I counted its public member function names. It has **45 distinct public
`basic_string` method names**, the same count as libstdc++: psychicstd is
missing `copy` but adds C++23's `contains`. See the
[`std::string` completeness case study](casestudies/string_completeness/stringcompletenesscasestudy.md)
for details. A present method name does not imply full compliance — see
[compliance.md](compliance.md) for behavioral coverage.

Some facilities are omitted deliberately when their practical value does not
justify the compile-time cost. For example, `<iostream>` provides the commonly
used narrow streams, but not `wcin`, `wcout`, `wcerr`, or `wclog`. Defining
those rarely used wide streams made a representative program about 6–7% slower
to compile even when it did not use them. Such gaps are preferred over making
every user pay for unused compliance.

## Why?

Slow compilation is one of the pain points of C++. Modules are supposed to
help, but they are not ready yet. Even if modules solve slow header parsing,
standard libraries are typically optimized for runtime performance. psychicstd
does not care about runtime performance — it is all about compilation speed.

I got the idea when I read about the [pystd](https://github.com/jpakkane/pystd) project from Jussi Pakkanen, which has completely different goals but got me thinking.

Writing a standard library is a massive undertaking with lots of corner cases.
Three factors in combination make it possible anyway:

- AI-assisted coding
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

[Implementation tricks](docs/implementation-tricks.md) collects the recurring
techniques used to reduce frontend work and keep Debug builds pleasant to step
through.

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
- **Sufficiently compliant with the C++ standard** — it should be correct enough to be useful. For example, `std::string` does not need to use small string optimization, which simplifies the implementation.
- **Support C++20**

Performance is measured primarily from debug builds of the real-world projects on the current Debian Stable compiler (GCC 14).

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

Unit tests should pass. The compliance test need not; 100% compliance is
unrealistic.

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
psychicstd-specific flags. The overlay requires CMake 3.26 or newer.

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

### CMake FetchContent

Projects that can link psychicstd explicitly may fetch and build it as a normal
static-library dependency:

```cmake
include(FetchContent)
FetchContent_Declare(
    psychicstd
    GIT_REPOSITORY https://github.com/pauldreik/psychicstd.git
    GIT_TAG main
)
FetchContent_MakeAvailable(psychicstd)

target_link_libraries(your_target PRIVATE psychicstd::psychicstd)
```

Pin `GIT_TAG` to a specific psychicstd commit when reproducible dependency
resolution is important. Tests and benchmarks default to off when psychicstd is
fetched. Linking the target supplies the replacement headers, static runtime,
and required ABI-library flags.

Every target containing C++ sources, including fetched dependencies, must use
the same standard library. Link `psychicstd::psychicstd` to each such target or
use the toolchain overlay when replacing the standard library for an untouched
project or its whole dependency graph.

### Meson native file

Meson native files provide equivalent whole-project integration for GCC 13+
and Clang. After building the psychicstd runtime as described above, create
`psychicstd.ini`:

```ini
[binaries]
cpp = 'g++-14'

[built-in options]
cpp_std = 'c++20'
cpp_args = ['-nostdinc++', '-fvisibility=hidden', '-isystem', '/path/to/psychicstd/include']
cpp_link_args = ['-nostdlib++', '/path/to/psychicstd/build-runtime/libpsychicstd.a', '-lsupc++', '-latomic']
```

Configure and build the project with that native file:

```bash
meson setup build-with-psychic --native-file psychicstd.ini
meson compile -C build-with-psychic
```

The native file applies to every C++ target in the host build, including
subprojects. As with the manual CMake integration, all linked C++ code must use
the same standard library. Meson caches compiler settings at setup time, so
reconfigure or recreate the build directory after changing the native file.
The [HarfBuzz benchmark](use_on_realworld_projects/harfbuzz_speed_report.md)
exercises this whole-project replacement through a Meson build.

### Using with Conan

If you already use Conan, the intended integration point is a single overlay
profile: [`conan/psychic.profile`](conan/psychic.profile).
It composes with your existing host profile and injects the psychicstd toolchain
without selecting a compiler or adding dependency-specific flags.

Apply the overlay to the host context so your application and its linked C++
dependencies use the same standard library. You can compose the profiles
directly on the command line. Conan applies repeated host profiles in order, so
put the psychicstd overlay last:

```bash
conan install . \
    --profile:host=your-host.profile \
    --profile:host=/path/to/psychic.profile \
    --profile:build=your-build.profile \
    --build=missing
```

Alternatively, create a wrapper host profile, for example
`psychic-host.profile`, that includes both profiles in the same order:

```text
include(your-host.profile)
include(/path/to/psychic.profile)
```

Then pass that single composed host profile to Conan:

```bash
conan install . \
    --profile:host=psychic-host.profile \
    --profile:build=your-build.profile \
    --build=missing
```

The host profile controls the application and the libraries linked into it. The
build profile controls tools that run during the build, so it normally remains
unchanged.

The example in [`tests/conan_project/`](tests/conan_project/) uses `fmt` to show
a real third-party dependency built this way. The profile does not overwrite
sanitizer flags, so ASan and UBSan keep working the way Conan or your project
already configures them. Supported compilers are the same as the
toolchain-overlay path: Clang and GCC 13+ on Linux.

The overlay includes the psychicstd release version in Conan package IDs, so
Conan does not confuse binaries built with psychicstd with those built with the
normal standard library. For local experiments, consider setting `CONAN_HOME`
to a separate directory to keep psychicstd-built packages out of your normal
Conan cache.

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

Correctness in practice is verified by compiling — and running the test suites of — actual third-party projects against psychicstd.
This is what is used to decide what to implement in the library — there is no
need to make it slower by implementing things no one uses (yes, I am talking
about you, valarray).

### Benchmarks

Benchmarks are central to reasoning about performance. Claims have to be backed
up!

The benchmarks are listed in order of importance:

| name | command | what it measures |
|---|---|---|
| Real-world projects | [`scripts/benchmark_realworld.py`](scripts/benchmark_realworld.py) | Complete third-party project builds with the platform standard library and psychicstd. This is the primary benchmark. |
| Header compile cost | [`scripts/benchmark_header_cost.py`](scripts/benchmark_header_cost.py) | Every public header in isolation, including strict and drop-in modes. This helps locate parsing and transitive-include costs. Results are written to [include_weight.md](include_weight.md). |
| Focused compile time | [`scripts/benchmark_compile_time.py`](scripts/benchmark_compile_time.py) | Stable example and focused workloads, measuring both elapsed compilation time and peak compiler RSS. Results are written to [compile_time.md](compile_time.md). |
| Startup time | `cmake -B build/ -S . -DCMAKE_BUILD_TYPE=Debug`<br>`cmake --build build/ --target startup_bench` | Exec-to-exit time for a representative small program. Results are written to [startup.md](startup.md). |

Run [`./run_benchmarks.sh QUICK`](run_benchmarks.sh) on an otherwise idle
machine for Debug-only real-world measurements with an estimated one-hour time
budget. [`./run_benchmarks.sh FULL`](run_benchmarks.sh) measures both Debug and
Release with a six-hour budget, capped at nine repetitions per project. Both
modes regenerate every report and disable ccache automatically.

Compiler memory matters because it controls how many compiler jobs can run in parallel and whether a development build starts swapping. In an uncached GCC 14 measurement, psychicstd used roughly half the peak compiler memory:

<!-- README_BENCHMARK:benchmark-peak-rss:start -->

| workload | libstdc++ peak RSS | psychicstd peak RSS | reduction |
|---|---:|---:|---:|
| [wordcounter](examples/wordcounter.cpp) | 114.5 MiB | 52.0 MiB | 55% |
| [`<iostream>` test](benchmarks/compile_time/bench_iostream.cpp) | 71.6 MiB | 39.2 MiB | 45% |

<!-- README_BENCHMARK:benchmark-peak-rss:end -->

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

Psychicstd prioritizes getting real projects running (that is: compiling and passing their unit tests) rather than maximizing the score on the libc++ tests. Run [`tools/compliance.py`](tools/compliance.py) with:

```bash
tools/compliance.py
```

It samples up to 15 libcxx test files per header, runs every selected test
against both the system STL and psychicstd, and updates `compliance.md`. The
summary table shows confirmed psychicstd passes across the complete relevant
test corpus, along with sampling progress and explicitly ignored
libc++-specific tests. A second table breaks tested cases into four groups:
both libraries pass, only libstdc++ passes, only psychicstd passes, or both
fail.

See the results in [compliance.md](compliance.md).

The same suite can be run under AddressSanitizer and UndefinedBehaviorSanitizer
([`tools/compliance.py`](tools/compliance.py) `--sanitize`) to catch memory bugs, UB, and
behavioral divergence, gated against a baseline of known failures. See
[docs/sanitizer-testing.md](docs/sanitizer-testing.md).
