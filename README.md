# psychicstd - a C++ standard library implementation

This is a **higly experimental** C++ standard library optimized for compilation speed. It is intended to be used during general C++ development to speed up the edit-compile-debug cycle. It is not at all intended to be used for shipping binaries.

It is not complete. It is not fully compliant. But it is good enough to quickly iterate on code. As an example, it took about five hours to go from nothing to **good enough to use for compiling rapidjson and Catch2**.

Here are some examples of compilation speed for real programs compared to gcc 12 in debug mode on amd64:

- rapidjson: 1.8x-2.4x speedup
- a unit test using catch2: 2.9x - 3.9x speedup

Once you have coded for a while, switch to a real quality standard library (typically libstdc++ or libc++) and test and build real releases - psychicstd is just intended for speedy develop.

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

Unit tests should pass. The compliance test need not, getting 100% compliance is utopic.

## Use in your project

No changes to your source or `CMakeLists.txt` are needed. Just pass these flags at configure time for your project:

```bash
cmake -S . -B build-with-psychic \
    -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -I/path/to/psychicstd/include"
```

To switch back to the system STL, configure without those flags. See `tests/external_project/run.sh` for a self-contained example.

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
