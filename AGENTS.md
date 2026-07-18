# Working on psychicstd

psychicstd is an experimental, Linux-only C++ standard-library replacement.
Its priority is faster compilation during development, followed by useful
behavior and freedom from undefined behavior. It is not a production runtime
library, does not replace the developer's normal standard library, and does
not aim for ABI stability or portable platform support. Its goal is to be
useful during the edit-compile-debug cycle.

## Scope and design

- Keep implementations small and easy for the compiler to parse and
  instantiate. Do not add complexity merely to match an implementation detail
  of libstdc++.
- Add comments when they clarify an otherwise non-obvious decision or
  constraint; keep them terse.
- Prefer standards-compatible behavior when it is cheap. Record deliberate
  gaps in `compliance.md` rather than disguising them as complete support.
- Preserve the distinction between compatibility modes. Default/drop-in mode
  may mirror libstdc++ transitive includes; strict mode must remain
  self-contained.
- The supported source language is C++20. Do not lower it for dependencies.
- GCC 12 remains supported, but prioritize compile-time performance on newer
  compilers, especially GCC 14. It is acceptable to use compiler-version
  preprocessor branches to make newer GCC versions faster, provided a GCC 12
  fallback remains available.

## Toolchains and linking

- Native project builds support GCC 12 and Clang on Linux. The distributable
  CMake toolchain (`cmake/psychicstd-toolchain.cmake`) supports Clang and GCC
  13+ because it uses `-nostdlib++`.
- Do not select a compiler in the psychicstd CMake toolchain. It is an overlay
  and must compose with a user, preset, or Conan-generated toolchain.
- Preserve user flags, especially `-fsanitize=address` and
  `-fsanitize=undefined`. Toolchain changes must not replace CMake flag
  variables wholesale.
- Code built with psychicstd must not accidentally link libstdc++. Keep the
  `-nostdinc++`, `-nostdlib++`/GCC-12 fallback, `-lsupc++`, and
  `-fvisibility=hidden` arrangements coherent.

## Conan integration

- `tests/conan_project/psychic.profile` is the user-facing Conan overlay.
  It is composed after normal host and build profiles and must not choose a
  compiler or add application-specific workarounds.
- Apply the profile to both host and build contexts when dependencies compile
  C++; mixing libstdc++-built C++ dependencies with psychicstd is unsupported.
- Keep the `fmt` example realistic: it should exercise the normal fmt build
  and a locale-enabled formatting call, not define `FMT_USE_LOCALE=0`.

## Validation

- Before compiling or running tests in parallel, consider both the available
  memory and CPU count. On machines with many cores and limited RAM, cap
  parallelism to preserve at least 1.5 GiB of available memory per active job.
- For builds and tests where compile-time measurement is not the purpose,
  prepend `/usr/lib/ccache/` to `PATH` and enable ccache to speed up repeated
  compilations. Disable ccache for compile-time benchmarks and performance
  measurements so cached results do not skew timings.
- Add a focused test in `tests/test_<header>.cpp` for changed public behavior.
  Tests are built both against the system library and psychicstd; both must
  pass.
- For local CI-equivalent coverage, use `./run_ci_locally.sh` (or a focused
  filter such as `./run_ci_locally.sh clang asan`). Use Docker for the GCC
  version matrix when available.
- For CMake toolchain changes, run the external-project tests, including ASan
  and UBSan variants. For Conan changes, run `tests/conan_project/run.sh` when
  Conan and its remotes are available.
- For fmt, verify the real-world recipe with:
  `python3 scripts/compare_realworld_performance.py --build-type release --reps 1 --project fmt`.

## Repository hygiene

- Keep changes distinct. Prefer small, focused commits over mixed refactors.
  Use a terse, specific commit subject and put rationale or implementation
  detail in the commit body.
- Use the repository formatting scripts relevant to files you touch:
  `run_clang_format.sh`, `run_cmake_format.sh`, `run_markdown_format.sh`,
  `run_python_lint.sh`, `run_shell_format.sh`, and `run_yaml_format.sh`.
- Do not commit generated build directories, `CMakeUserPresets.json`, Conan
  output, Python `__pycache__`, benchmark downloads, or speed reports unless
  the task explicitly asks for generated results.
- Keep unrelated working-tree changes intact. Do not use destructive Git
  commands to obtain a clean tree.
