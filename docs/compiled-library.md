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

## Follow-up experiments

### Cold-path static library

This experiment compares against committed baseline `f8c842d`. It started
from that commit with staged PIC changes in `CMakeLists.txt` and the toolchain
validation overlay, plus a staged benchmark-harness change which builds the
existing `iostream.cpp` runtime. The worktree also contained a staged README
speed-table refresh and the untracked
`scripts/rewrite_commit_preserving_merges.py` from an interrupted full report
generator run. Those report artifacts are pre-existing and are excluded from
this experiment; the full speed-report set will not be regenerated or
overwritten here.

Measurements use GCC 14.2.0, C++20 Debug (`-O0 -g`), `-nostdinc++`,
`-fvisibility=hidden`, and `-fPIC`. Ccache is disabled. This host has 20 CPUs
and initially had about 29 GiB available memory, so validation at that point
used at most 19 parallel jobs. The focused measurements are serial. Each
compile-time result is the median of three compiler-cache-free repetitions.
GCC `-ftime-report` totals are recorded for the median repetition.

The focused probes produced these results. `Parse`, `tmpl`, and `codegen` are
the GCC report's wall-clock phase parsing, template instantiation, and
optimization/code-generation totals; their 10 ms resolution makes zeroes and
small changes directional rather than precise.

| Slice and probe | Header bytes | Probe object bytes | Elapsed | Parse / tmpl / codegen |
| --- | ---: | ---: | ---: | ---: |
| Exceptions include, before | 2,217 | 9,304 | 0.14 s | 0.13 / 0.01 / 0.00 s |
| Exceptions include, after | 2,544 | 9,304 | 0.14 s | 0.13 / 0.01 / 0.00 s |
| Exceptions exercised, before | 2,217 | 22,840 | 0.15 s | 0.13 / 0.00 / 0.01 s |
| Exceptions exercised, after | 2,544 | 10,864 | 0.14 s | 0.13 / 0.00 / 0.01 s |
| System error include, before | 6,981 | 49,368 | 0.17 s | 0.15 / 0.00 / 0.01 s |
| System error include, after | 4,781 | 13,752 | 0.15 s | 0.14 / 0.01 / 0.00 s |
| System error exercised, before | 6,981 | 105,240 | 0.21 s | 0.15 / 0.00 / 0.04 s |
| System error exercised, after | 4,781 | 15,848 | 0.15 s | 0.14 / 0.00 / 0.01 s |
| String include, before | 35,298 | 9,240 | 0.14 s | 0.13 / 0.01 / 0.01 s |
| String include, after | 35,064 | 9,240 | 0.14 s | 0.13 / 0.01 / 0.01 s |
| String conversions, before | 35,298 | 69,704 | 0.17 s | 0.13 / 0.00 / 0.03 s |
| String conversions, after | 35,064 | 40,104 | 0.14 s | 0.13 / 0.01 / 0.01 s |

The new Clang Debug runtime objects are 53,680 bytes for `stdexcept.cpp`,
77,392 bytes for `system_error.cpp`, and 59,520 bytes for `string.cpp`. The
exception slice is kept because exercised code generation fell substantially
without an include-only cost. The system-error slice is kept: its exercised
object fell by 85% and its median compile fell by 29%. The string slice is
kept: both probes avoid conversion code generation, with the exercised compile
down about 18%. Focused behavior tests pass against both the system library and
psychicstd.

The first fmt run caught an accidental compatibility change: removing
`<cstdio>` and `<string.h>` from `<system_error>` also removed names fmt uses
from the drop-in transitive include surface. They are retained in drop-in mode
but not strict mode. `<string>` similarly retains `<cerrno>` and `<cstdlib>` in
drop-in mode, while strict `-fno-exceptions` includes them for its local
conversion-and-abort fallback. The corrected focused string numbers above were
remeasured from three repetitions.

The combined GCC 14 Debug real-project comparison used the harness-selected 20
jobs; available memory was at least 30 GiB during those runs. Every result was
statistically within noise, and no project regressed by 3%:

| Project and phase | Baseline | Combined | Change | System drift |
| --- | ---: | ---: | ---: | ---: |
| rdfind compile | 243.6 ms | 236.2 ms | -3.1% | +0.9% |
| fmt compile | 4,853.3 ms | 4,825.2 ms | -0.6% | +2.2% |
| Catch2 compile | 3,025.4 ms | 2,917.7 ms | -3.6% | +0.7% |
| Eigen compile | 11,066.8 ms | 11,016.2 ms | -0.5% | +2.8% |

The associated test phases changed by +0.1%, +1.8% (2.3 ms), -0.1%, and
+1.8% (0.4 ms), respectively. The combined result therefore passes the gate,
and all three slices are kept.

Validation covered the affected public behavior against both libstdc++ and
psychicstd with GCC 14 and Clang, strict self-contained headers, focused ASan
and UBSan runs, and the external-project toolchain tests with ASan, UBSan, and
both combined. A complete runtime archive also builds with `-fno-exceptions`;
optional and string-conversion failure consumers link and abort locally. A
normal executable and a shared-library consumer both link the GCC 14 PIC
archive successfully.
