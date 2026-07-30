# Working on psychicstd

psychicstd is an experimental C++ standard-library replacement for the
edit-compile-debug cycle. Its priorities are usefulness in real projects,
freedom from undefined behavior, and faster debug compilation. It is not a
production runtime library, does not replace the developer's normal standard
library, and does not aim for ABI stability. Linux is the primary platform;
macOS 14.4 and newer is also supported.

## Scope and design

- Keep implementations small and easy for the compiler to parse and
  instantiate. Do not add complexity merely to match an implementation detail
  of libstdc++.
- Add comments when they clarify an otherwise non-obvious decision or
  constraint; keep them terse.
- Prefer standards-compatible behavior when it is cheap. Do not add expensive
  machinery merely to claim completeness. Document intentional limitations
  near the implementation or in stable documentation when useful.
- `compliance.md` and `compliance.sanitize.md` are generated reports. Regenerate
  them with `tools/compliance.py`; do not edit them by hand.
- Preserve the distinction between compatibility modes. Default/drop-in mode
  may mirror libstdc++ transitive includes; strict mode must remain
  self-contained.
- C++20 is the supported language baseline. Consumers and dependency tests may
  use newer language modes.
- Native builds support GCC 12 through 16 and Clang on Linux, plus AppleClang
  on macOS 14.4 and newer. Prioritize compile-time performance in GCC 14 debug
  builds, which are the main published baseline. Compiler-version branches are
  acceptable when they preserve supported fallbacks.

## Toolchains and linking

- The distributable CMake toolchain
  (`cmake/psychicstd-toolchain.cmake`) supports GCC 13+ and Clang on Linux,
  and AppleClang on macOS. GCC 12 is supported by native builds and the manual
  `-nodefaultlibs` integration, not by the distributable overlay.
- Do not select a compiler in the psychicstd CMake toolchain. It is an overlay
  and must compose with a user, preset, or Conan-generated toolchain.
- Preserve user flags, especially `-fsanitize=address` and
  `-fsanitize=undefined`. Toolchain changes must not replace CMake flag
  variables wholesale.
- Code built with psychicstd must not accidentally link libstdc++. Keep the
  `-nostdinc++`, `-nostdlib++`/GCC-12 fallback, `-lsupc++`, and
  `-fvisibility=hidden` arrangements coherent.
- When adding or removing runtime `.cpp` files, update the canonical
  `cmake/psychicstd-runtime-sources.txt` manifest. The top-level build,
  distributable toolchain, and real-world benchmark driver all read it.

## Conan integration

- `conan/psychic.profile` is the user-facing Conan overlay.
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
  filter such as `./run_ci_locally.sh clang asan`). It covers native GCC and
  Clang debug/release builds, Clang sanitizers, strict compatibility, and the
  GCC 12/14/16 Docker matrix when Docker is available.

- For CMake toolchain changes, run the external-project tests, including ASan
  and UBSan variants. For Conan changes, run `tests/conan_project/run.sh` when
  Conan and its remotes are available.

- For fmt, verify the real-world recipe in debug mode:

  ```bash
  scripts/benchmark_realworld.py \
      --project fmt --build-type debug --reps 1 --check-only
  ```

- Treat unexpectedly slow tests as possible algorithmic bugs. As a rule of
  thumb, inspect a test that takes more than one second and more than twice as
  long as the system-library build; avoid accidental quadratic behavior.

## Performance measurement

- Real-world debug builds are the primary benchmark. Header cost, focused
  compile-time/peak-RSS workloads, and startup time are supporting diagnostics.
- Use `./run_benchmarks.sh QUICK` on an otherwise idle machine for the
  one-hour, Debug-only real-world plan, or `./run_benchmarks.sh FULL` for both
  Debug and Release. Both regenerate all checked-in reports and disable ccache.
- Use `./run_benchmarks.sh INACCURATE_BUT_FAST` only to smoke-test the benchmark
  machinery. It minimizes repetition counts and enables ccache, so its results
  are not meaningful performance measurements.
- Use `scripts/compare_compile_time_to_ref.py` and
  `scripts/compare_realworld_to_ref.py` to check a branch for regressions
  against another psychicstd revision.
- Do not infer performance from implementation complexity alone. Measure before
  and after, disable ccache, record the compiler/build mode/job count, and
  investigate regressions.

## Repository hygiene

- Never open or close a pull request without the user's explicit consent for
  that action.
- Keep changes distinct. Prefer small, focused commits over mixed refactors.
  Use a terse, specific commit subject and put rationale or implementation
  detail in the commit body. Wrap commit body lines at 80 columns.
- When a change logically belongs to an earlier unmerged commit, use a fixup
  and autosquash it instead of adding a follow-up commit.
- Use the repository formatting scripts relevant to files you touch:
  `run_clang_format.sh`, `run_cmake_format.sh`, `run_markdown_format.sh`,
  `run_python_lint.sh`, `run_shell_format.sh`, and `run_yaml_format.sh`.
- Invoke executable repository scripts directly and rely on their shebangs.
  Specify `python3` only when deliberately overriding the interpreter.
- Do not commit generated build directories, `CMakeUserPresets.json`, Conan
  output, Python `__pycache__`, or benchmark downloads. Commit generated
  benchmark/compliance reports only when the task includes regenerating them.
- Keep unrelated working-tree changes intact. Do not use destructive Git
  commands to obtain a clean tree.
