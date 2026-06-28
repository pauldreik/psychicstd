# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

**psychicstd** is an experimental, AI-assisted C++ standard library implementation (header-only) optimized for compilation speed over runtime performance. It replaces system libstdc++ headers via `-nostdinc++` to achieve 1.8x–3.9x faster compile times. Supports GCC 12–15 on Linux x86-64 only.

## Build & Test

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run a single test by name:

```bash
ctest --test-dir build -R test_vector_psychicstd --output-on-failure
```

Compile-time benchmarks (requires `build` target first):

```bash
cmake --build build --target bench
# or directly:
python3 benchmarks/compile_time/run_bench.py
```

Standards compliance testing (requires an llvm-project checkout):

```bash
python3 tools/compliance.py            # all headers
python3 tools/compliance.py map        # one header
python3 tools/compliance.py --sample 50 vector  # 50 new sampled tests
```

## Formatting

Run these before committing. Each script uses the config files in the repo root:

```bash
./run_clang_format.sh    # C++ headers (.clang-format: LLVM style, left pointer alignment)
./run_cmake_format.sh    # CMake files (.cmake-format.py + .gersemirc, line_width=100)
./run_python_lint.sh     # Python (ruff)
./run_shell_format.sh    # Shell scripts (shfmt)
```

## Architecture

**Library type:** Header-only `INTERFACE` CMake target (`psychicstd::psychicstd`).

**How it works:** Sets `-nostdinc++` and adds `include/` to the include path, so the 56 headers in `include/` shadow system libstdc++ headers. GCC 12–15 requires explicit linking of `libsupc++`, `libm`, `libc`, `libgcc_s`/`libgcc` (handled in `CMakeLists.txt`).

**Three-level test structure:**

1. `tests/` — unit tests. Each header is compiled twice: once with system headers (`test_*_system`) and once with psychicstd (`test_*_psychicstd`). Both must pass to verify behavioral equivalence.
1. `tests/external_project/` — simulates an independent CMake project consuming psychicstd via injected flags.
1. `validation/` — compiles real third-party code (rapidjson, Catch2 v3.8.0) with psychicstd headers.

**Key implementation notes:**

- No small-string optimization in `string` (deliberate — keeps it simple and fast to compile).
- `vector` uses a `__vec_iter` template wrapper for iterator support.
- All implementation lives in the `std::` namespace; no `psychicstd::` namespace.
- Non-goals: runtime performance, portability, ABI stability, pre-C++20 compatibility.

## Using psychicstd in an External Project

**CMake** — no source changes needed, inject flags at configure time:

```bash
cmake -S . -B build \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -isystem /path/to/psychicstd/include"
```

**Autoconf/make** — bake flags into the Makefile at configure time so plain `make` works:

```bash
./configure \
    CXXFLAGS="-std=c++20 -nostdinc++ -isystem /path/to/psychicstd/include" \
    LDFLAGS="-nodefaultlibs" \
    LIBS="-lsupc++ -lm -lc -lgcc_s -lgcc"
make
```

## CMake Options

| Option | Default | Effect |
|---|---|---|
| `PSYCHICSTD_BUILD_TESTS` | ON | Unit tests |
| `PSYCHICSTD_BUILD_BENCHMARKS` | ON | Compile-time benchmarks |
| `PSYCHICSTD_BUILD_VALIDATION` | ON | rapidjson + Catch2 validation |
