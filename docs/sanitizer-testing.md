# Sanitizer testing

How to run the libc++ test suite against psychicstd under AddressSanitizer and
UndefinedBehaviorSanitizer, read the results, and keep the gate green while
fixing bugs.

## Why

We don't hand-write a unit suite (the standard library's API surface makes that
cost more than the library itself). Instead we **borrow** the libc++ test suite
and run it under sanitizers, using the system STL (libstdc++) as an oracle:

- A test that **passes on libstdc++ but fails under psychicstd** is a psychicstd
  bug — nothing else to decide.
- Under ASan/UBSan this catches three classes at once: **memory bugs** (ASan),
  **undefined behavior** (UBSan), and **behavioral divergence** (the libc++
  tests' own `assert`s abort when psychicstd does the wrong thing).

This is the same machinery as `tools/compliance.py`; `--sanitize` just adds the
sanitizer flags, actually runs the executables, and gates on a baseline.

## Prerequisites

- `g++` with ASan/UBSan (the default on Linux).

- A libc++ test checkout. A sparse clone is enough (this is what CI does):

  ```bash
  git clone --depth 1 --filter=blob:none --sparse \
      https://github.com/llvm/llvm-project.git llvm-project
  git -C llvm-project sparse-checkout set libcxx/test
  ```

- Point the tool at it (or put it at the default `~/code/thirdparty/llvm-project`):

  ```bash
  export LLVM_ROOT=$PWD/llvm-project
  ```

## Running it

```bash
# Full sweep, all headers (what CI runs)
python3 tools/compliance.py --sanitize

# One or more headers
python3 tools/compliance.py --sanitize unordered_set deque

# More tests per header (default is 15)
python3 tools/compliance.py --sanitize --sample 40 vector
```

`--sanitize` uses its own cache (`.compliance_cache.sanitize.json`) and output
(`compliance.sanitize.md`), both git-ignored, so it never disturbs the normal
`compliance.md` run.

The sampled set is deterministic for a given `--sample` and header set (fixed
RNG seed, fresh cache), so local and CI runs pick the same tests.

## Reading the results

Each test lands in one of three states (only tests that pass on libstdc++ are
run against psychicstd):

| state | meaning |
|-------|---------|
| `pass` | compiles and runs clean under both STLs |
| `cfail` | does not compile with psychicstd (an **unimplemented** feature, not a bug we gate on) |
| `rfail` | compiles but the run fails under psychicstd — sanitizer abort, crash, or a failed `assert`. **This is the bug signal.** |

The gate at the end prints, relative to the baseline:

- **known** — `rfail`s already in the baseline (listed, not fatal).
- **NEW** — `rfail`s not in the baseline → exit non-zero (**CI goes red**).
- **now PASS** — baselined tests that started passing → nudge to drop them.

## The baseline

`tools/compliance_sanitize_baseline.txt` lists the known failures (paths
relative to `libcxx/test`). CI fails **only on failures not in this list**, so
the existing backlog is tracked-but-not-blocking, exactly like the compile-time
perf-diff and compliance ratchets. Never let it grow silently — a new entry
means a new bug slipped in.

## Workflow: fixing a baselined bug

1. Pick a line from `tools/compliance_sanitize_baseline.txt` and reproduce it by
   hand (see below).

1. Fix the header.

1. Re-run that header and confirm it now passes:

   ```bash
   python3 tools/compliance.py --sanitize --recheck unordered_set
   ```

   The gate will report it under **now PASS**.

1. Refresh the baseline and commit it with the fix:

   ```bash
   python3 tools/compliance.py --sanitize --update-baseline
   git add tools/compliance_sanitize_baseline.txt
   ```

## Workflow: a NEW failure appears (CI red)

A `NEW` entry is a regression or a newly-covered bug. Either **fix it** (the
default — bugs before features) or, if it's a genuinely accepted limitation,
add it to the baseline with `--update-baseline` and say why in the commit
message. Don't baseline a real bug just to get green.

## Reproducing one test by hand

The fastest debug loop — compile a single libc++ test against psychicstd with
the same flags the tool uses, then run it:

```bash
INC=include
SUP=$LLVM_ROOT/libcxx/test/support
T=$LLVM_ROOT/libcxx/test/std/containers/unord/unord.set/erase_const_iter.pass.cpp

g++ -std=c++23 -fsanitize=address,undefined -fno-sanitize-recover=all -g \
    -nostdinc++ -isystem "$INC" -I"$SUP" "$T" -o /tmp/t
ASAN_OPTIONS=abort_on_error=1 UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 /tmp/t
```

Swap `-isystem "$INC"` for nothing (and drop `-nostdinc++`) to see it pass on
libstdc++ — that difference is the bug.

## CI

`.github/workflows/compliance-sanitizers.yml` runs the full sweep on push to
`main`, weekly, and on manual dispatch. It sparse-clones `libcxx/test`, runs
`--sanitize`, and fails only on `NEW` failures.

## Caveat

The baseline is compiler/environment sensitive: a different `g++` version can
change which tests pass. Regenerate it in an environment close to CI, and if CI
reports env-only differences, reconcile with `--update-baseline`.
