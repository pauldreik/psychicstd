# psychicstd - a C++ standard library implementation

This is a **higly experimental** C++ standard library optimized for compilation speed. It is intended to be used during general C++ development to speed up the edit-compile-debug cycle. It is not at all intended to be used for shipping binaries.

It is not complete. It is not fully compliant. But it is good enough to quickly iterate on code. Here are some real world projects that compile with psychicstd. The number indicate the speedup relative libstdc++:

- catch2 [1.6x](use_on_realworld_projects/catch2_speed_report.md)
- cppcheck [2.3x](use_on_realworld_projects/cppcheck_speed_report.md)
- eigen [1.7x](use_on_realworld_projects/eigen_speed_report.md)
- fmt
- nlohmann json
- rdfind

Find the scripts validating the build and generating the above number in the use_on_realworld_projects/ directory.

Once you have coded for a while, switch to a real quality standard library (typically libstdc++ or libc++) and test and build real releases - psychicstd is just intended for speedy develop.

## How complete is it?

Completeness varies by header, development has been guided what is needed to get realworld
projects to compile. The `std::string` header is perhaps the most used header and to
investigate the complementess I counted the number of public member functios to see what
is missing. It implements **44 of libstdc++'s 45 public `basic_string` method names (98%)**, missing only `copy` and `get_allocator`. See the [`std::string` completeness case study](casestudies/string_completeness/stringcompletenesscasestudy.md) for details. Note that a present method name is not full compliance — see [compliance.md](compliance.md) for behavioral coverage.

## Why?

Slow compilation is one of the pain points of C++. Modules are supposed to help, but it is not yet ready. Also, even if modules solve the slow parsing of include files problem, standard libraries are typically optimized for runtime performance. Psychichstd does not care about runtime performance - it is all about compilation speed.

I got the idea when I read about the [pystd](https://github.com/jpakkane/pystd) project from Jussi Pakkanen, which has completely different goals but got me thinking.

Writing a standard library is a massive undertaking and super hard with lots of corner cases. Three factors in combination makes it possible anyway:

- AI assisted coding
- the excellent libc++ test suite
- no demands on portability, runtime performance, ABI stability etc

An AI can mostly guide itself trying to get the library through all the tests. Anything the AI can generate that passes the test suite is useful.

## How does it work?

Psychicstd is so far only used with gcc. It passes `-nostdinc++ -I/path/to/psychicstd/include` to the compiler. That causes the compiler to pick up vector, string and other headers from psychicstd instead of the standard library that comes with gcc (libstdc++).

The standard library uses `std::` namespace just like the real standard. You should not have to do any changes to your program.

## Why is it faster?

Compile time for these headers is dominated by the compiler *frontend*, in two parts: **parsing declarations** and **instantiating templates**. psychicstd wins by having far less of both. Raw byte count, number of include files, and backend code generation are all second-order. Precompiled headers help by caching that same frontend work — but they attack the same bottleneck, so psychicstd without a PCH is roughly as fast as libstdc++ with one, and still wins when both use PCH. The [case studies](casestudies/) measure each of these effects.

## The name

The name is a word play on the edit-compile-debug cycle itself: psychic → cycle → cyclic. A psychic knows the answer before you've finished asking the question. psychicstd tries to do the same — by the time you've hit save, the compiler is already done. Well, that's the aspiration, anyway.

## Goals

These are the goals, in order

- **Sufficiently compliant to the C++ standard** — it should be correct enough to be useful. For example, `std::sort` just needs to sort — it doesn't need to be O(N log N). `std::string` does not need to use small string optimization, which simplifies the implementation.
- **Faster compilation** — If it is not faster to compile than real implementations like libstdc++, this project has absolutely no value.
- **Useful in practice** — code should compile, link, and run well enough to support a normal development workflow.
- **Support C++20**

## Non-goals

- Runtime performance — we don't try to make your program run faster, only to compile faster.
- Compilation speed in release mode
- Portability — Developed on and for gcc/Linux/x86-64. Might work with clang and arm as well, but that is untested. There is no windows or MSVC support.
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

### CMake / GCC 12

GCC 12 lacks `-nostdlib++`, so you need `-nodefaultlibs` and all the
libraries spelled out:

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_COMPILER_WORKS=1 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -fvisibility=hidden -isystem /path/to/psychicstd/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc"
```

`CMAKE_CXX_COMPILER_WORKS=1` skips the compiler detection link test (the test
program needs libc, but `CMAKE_CXX_STANDARD_LIBRARIES` isn't applied during
detection).

### CMake / GCC 13+

GCC 13 added `-nostdlib++`, which drops libstdc++ while keeping libc, libm,
libgcc_s and libgcc — only `-lsupc++` needs to be added back:

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -fvisibility=hidden -isystem /path/to/psychicstd/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-nostdlib++" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++"
```

No `CMAKE_CXX_COMPILER_WORKS` needed — libc is still linked by default.

### CMake / Clang

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -fvisibility=hidden -isystem /path/to/psychicstd/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-nostdlib++" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++"
```

### Notes for all configurations

`-fvisibility=hidden` prevents psychicstd's symbols from interposing with
libstdc++ at runtime (needed when sanitizers pull in `libstdc++.so`).
`-isystem` (rather than `-I`) suppresses warnings from psychicstd headers.

`CMAKE_CXX_STANDARD_LIBRARIES` places the libraries *after* the object files
on the link line. This is required because `-lsupc++` is a static archive
and the linker processes archives in order.

To switch back to the system STL, configure without these flags.
See `tests/external_project/run.sh` for a self-contained working example.

### Autoconf/make

Pass flags as `./configure` arguments so they are baked into the Makefile —
plain `make` then works without extra flags:

```bash
./configure \
    CXXFLAGS="-std=c++20 -nostdinc++ -isystem /path/to/psychicstd/include" \
    LDFLAGS="-nodefaultlibs" \
    LIBS="-lsupc++ -lm -lc -lgcc_s -lgcc"
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

Tests cover three things:

- each implemented header compiled against both the system STL and psychicstd
- a simulated external project that uses psychicstd via `-DCMAKE_CXX_FLAGS`
- a `validation/` directory where real third-party libraries are compiled against psychicstd and exercised

### Validation by testing to compile third party programs

`validation/` contains programs that compile third-party libraries against psychicstd and run them as ctest tests. The intent is to catch regressions and verify that psychicstd is correct enough to be useful in practice. Currently: rapidjson (parse, write, DOM manipulation) and Catch2 v3.8.0 (basic tests, algorithms, string operations).

### Benchmarks

Since this library is all about compilation speed, there is benchmarking to measure compilation speed.

```bash
cmake --build build --target bench
# or directly:
python3 benchmarks/compile_time/run_bench.py
```

View the results by inspecting [speed.md](speed.md).

For a deeper look at *why* psychicstd compiles faster, see the per-header case studies, which use clang's `-ftime-trace` to break down where the time goes:

- [`<string>` compilation speed case study](casestudies/std_string/stdstringcasestudy.md)
- [`<vector>` compilation speed case study](casestudies/std_vector/stdvectorcasestudy.md)
- [does the *number* of include files matter?](casestudies/include_overhead/includeoverheadcasestudy.md) — testing whether fewer, larger headers compile faster
- [template instantiation vs. raw byte count](casestudies/template_depth/templatedepthcasestudy.md) — showing instantiation work, not bytes, drives compile time
- [precompiled headers vs. psychicstd](casestudies/precompiled_headers/precompiledheaderscasestudy.md) — how PCH stacks with psychicstd, and whether psychicstd still wins
- [C++20 modules vs. psychicstd](casestudies/modules/modulescasestudy.md) — header units vs. PCH, and why modules close the gap the most

## Standards compliance

Psychicstd uses the libc++ unit tests to ensure the library is standards compliant. You need to check out the source code separately from [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project).

Running these tests takes a very long time - they are extensive! For that reason, by default only a random subset is used for each header.

```bash
python3 tools/compliance.py
```

It samples up to 15 libcxx test files per header, runs them against both the system STL (as a baseline filter) and psychicstd, and updates `compliance.md` with per-header results. Each header gets two indicators:

- **Compliance** — what fraction of the sampled libcxx tests pass: 🟢 ≥80%, 🟡 compiles but fewer pass, 🔴 does not compile with psychicstd.
- **Speed** — median compile time of the libcxx test files, psychicstd vs system: 🟢 >1.2×, 🟡 similar, 🔴 slower.

See the results in [compliance.md](compliance.md).
