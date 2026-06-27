#!/usr/bin/env python3
"""
Pick random libcxx conformance tests for a given header and run them
against both the system STL and psychicstd.

Only tests where the system STL (libstdc++) passes are considered — otherwise
the test is likely libc++-specific and not a meaningful signal.

Usage:
    libcxx_test.py <header> [--count N] [--seed S]
    libcxx_test.py array
    libcxx_test.py array --count 10
    libcxx_test.py string --count 5 --seed 42
    libcxx_test.py vector --all
"""

import argparse
import os
import random
import re
import subprocess
import sys
import tempfile
from pathlib import Path

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
REPO_ROOT = Path(__file__).parent.parent.resolve()
PSYCHICSTD = REPO_ROOT / "include"
LLVM_ROOT = Path.home() / "code/thirdparty/llvm-project"
LIBCXX_TEST = LLVM_ROOT / "libcxx" / "test"
SUPPORT_DIR = LIBCXX_TEST / "support"

# ---------------------------------------------------------------------------
# Header → test directory mapping
# ---------------------------------------------------------------------------
HEADER_TO_DIRS: dict[str, list[str]] = {
    "algorithm": ["algorithms"],
    "array": ["containers/sequences/array"],
    "cassert": ["language.support/support.runtime"],
    "cctype": ["strings/c.strings"],
    "cerrno": ["language.support/support.runtime"],
    "cfloat": ["language.support/support.limits"],
    "climits": ["language.support/support.limits"],
    "cmath": ["numerics/c.math"],
    "cstddef": ["language.support/support.types"],
    "cstdint": ["utilities/intseq"],
    "cstdlib": ["language.support/support.runtime"],
    "cstring": ["strings/c.strings"],
    "exception": ["language.support/support.exception"],
    "functional": ["utilities/function.objects"],
    "fstream": ["input.output/file.streams"],
    "iostream": ["input.output/iostream.objects"],
    "iterator": ["iterators"],
    "limits": ["language.support/support.limits/limits"],
    "map": ["containers/associative/map"],
    "memory": ["utilities/memory"],
    "numeric": ["numerics/numeric.ops"],
    "ostream": ["input.output/iostream.format/output.streams"],
    "random": ["numerics/rand"],
    "regex": ["re"],
    "set": ["containers/associative/set"],
    "sstream": ["input.output/string.streams"],
    "stack": ["containers/container.adaptors/stack"],
    "stdexcept": ["diagnostics/std.exceptions"],
    "streambuf": ["input.output/stream.buffers"],
    "string": ["strings/basic.string"],
    "string_view": ["strings/string.view"],
    "tuple": ["utilities/tuple"],
    "type_traits": ["utilities/meta"],
    "utility": ["utilities/utility"],
    "vector": ["containers/sequences/vector"],
}

# ---------------------------------------------------------------------------
# Lit marker parsing
# ---------------------------------------------------------------------------
_RE_UNSUPPORTED = re.compile(r"^//\s*UNSUPPORTED:\s*(.+)$", re.MULTILINE)
_RE_REQUIRES = re.compile(r"^//\s*REQUIRES:\s*(.+)$", re.MULTILINE)
_RE_ADD_FLAGS = re.compile(r"^//\s*ADDITIONAL_COMPILE_FLAGS:\s*(.+)$", re.MULTILINE)


def _tokens(line: str) -> list[str]:
    return [t.strip() for t in re.split(r"[,\s]+", line) if t.strip()]


def should_skip(path: Path) -> tuple[bool, str]:
    """Return (skip, reason)."""
    stem = path.stem

    # Only run .pass.cpp and .compile.pass.cpp
    if stem.endswith(".verify") or stem.endswith(".fail"):
        return True, "compile-failure test"
    if path.suffix != ".cpp":
        return True, "not a .cpp file"
    if not (stem.endswith(".pass") or stem.endswith(".compile.pass")):
        return True, f"unknown pattern '{stem}'"

    text = path.read_text(errors="replace")

    # UNSUPPORTED: skip if c++23 is explicitly unsupported
    for m in _RE_UNSUPPORTED.finditer(text):
        if "c++23" in _tokens(m.group(1)):
            return True, "UNSUPPORTED: c++23"

    # REQUIRES: skip libcpp-specific requirements
    for m in _RE_REQUIRES.finditer(text):
        toks = _tokens(m.group(1))
        bad = [t for t in toks if t.startswith("libcpp-")]
        if bad:
            return True, f"REQUIRES: {bad[0]}"

    return False, ""


def extra_flags(text: str) -> list[str]:
    flags = []
    for m in _RE_ADD_FLAGS.finditer(text):
        flags.extend(m.group(1).split())
    return flags


# ---------------------------------------------------------------------------
# Compile & run
# ---------------------------------------------------------------------------
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
DIM = "\033[2m"
BOLD = "\033[1m"
RESET = "\033[0m"

CXX = os.environ.get("CXX", "g++")


def run_cmd(cmd: list[str], timeout: int = 20) -> tuple[bool, str]:
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        return r.returncode == 0, (r.stdout + r.stderr).strip()
    except subprocess.TimeoutExpired:
        return False, f"timeout after {timeout}s"
    except FileNotFoundError as e:
        return False, str(e)


def try_compile_run(
    src: Path, flags: list[str], xflags: list[str], run_exe: bool
) -> tuple[str, str]:
    """Returns (status, first_error_lines). status: pass | fail | compile-fail"""
    with tempfile.NamedTemporaryFile(suffix="", delete=False, dir="/tmp") as f:
        exe = f.name
    try:
        ok, out = run_cmd(
            [
                CXX,
                "-std=c++23",
                *flags,
                *xflags,
                f"-I{SUPPORT_DIR}",
                str(src),
                "-o",
                exe,
            ]
        )
        if not ok:
            return "compile-fail", out
        if not run_exe:
            return "pass", ""
        ok, out = run_cmd([exe])
        return ("pass" if ok else "fail"), out
    finally:
        try:
            os.unlink(exe)
        except OSError:
            pass


def first_error(detail: str, n: int = 4) -> list[str]:
    """Return the first n non-empty lines of the compiler output."""
    lines = [line for line in detail.splitlines() if line.strip()]
    return lines[:n]


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def collect_tests(dirs: list[str]) -> list[Path]:
    tests: list[Path] = []
    for d in dirs:
        base = LIBCXX_TEST / "std" / d
        if not base.exists():
            print(
                f"  {YELLOW}warning: test dir not found: {base}{RESET}", file=sys.stderr
            )
            continue
        tests.extend(base.rglob("*.cpp"))
    return sorted(set(tests))


def main() -> None:
    p = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    p.add_argument("header", help="Header to test, e.g. 'array'")
    p.add_argument(
        "--count", type=int, default=5, help="Number of random tests (default: 5)"
    )
    p.add_argument(
        "--seed", type=int, default=None, help="RNG seed for reproducibility"
    )
    p.add_argument("--all", action="store_true", help="Run all eligible tests")
    args = p.parse_args()

    header = args.header.lstrip("<>")
    dirs = HEADER_TO_DIRS.get(header)
    if not dirs:
        known = sorted(HEADER_TO_DIRS)
        sys.exit(f"Unknown header '{header}'. Known: {', '.join(known)}")

    print(f"{BOLD}Testing <{header}>{RESET}  (psychicstd={PSYCHICSTD.name})")
    print(f"{DIM}compiler: {CXX}{RESET}\n")

    # Collect & pre-filter
    all_tests = collect_tests(dirs)
    eligible: list[Path] = []
    skipped: dict[str, int] = {}
    for t in all_tests:
        skip, reason = should_skip(t)
        if skip:
            skipped[reason] = skipped.get(reason, 0) + 1
        else:
            eligible.append(t)

    print(
        f"Candidates: {len(all_tests)} total, {len(eligible)} eligible, "
        f"{sum(skipped.values())} pre-filtered"
    )
    for reason, n in sorted(skipped.items(), key=lambda x: -x[1]):
        print(f"  {DIM}skip ({n}): {reason}{RESET}")
    print()

    if not eligible:
        sys.exit(f"{YELLOW}No eligible tests.{RESET}")

    # Random selection
    rng = random.Random(args.seed)
    sample = (
        eligible if args.all else rng.sample(eligible, min(args.count, len(eligible)))
    )
    sample.sort()

    sys_flags = []
    psy_flags = ["-nostdinc++", f"-I{PSYCHICSTD}"]

    # Counters
    n_sys_skip = 0  # system STL also fails → not a useful signal
    n_pass = 0  # both pass
    n_psy_fail = 0  # sys passes, psy fails (real gap)
    n_psy_warn = 0  # sys passes, psy has runtime failure

    for src in sample:
        rel = src.relative_to(LIBCXX_TEST / "std")
        is_exec = not src.stem.endswith(".compile.pass")
        xf = extra_flags(src.read_text(errors="replace"))

        sys_status, sys_detail = try_compile_run(src, sys_flags, xf, is_exec)

        if sys_status != "pass":
            # System STL couldn't compile/run this test — not a useful baseline
            n_sys_skip += 1
            print(f"  {DIM}skip (sys also fails): {rel}{RESET}")
            continue

        psy_status, psy_detail = try_compile_run(src, psy_flags, xf, is_exec)

        if psy_status == "pass":
            n_pass += 1
            print(f"  {GREEN}pass{RESET}  {rel}")
        elif psy_status == "compile-fail":
            n_psy_fail += 1
            print(f"  {RED}CFAIL{RESET} {rel}")
            for line in first_error(psy_detail):
                print(f"        {DIM}{line}{RESET}")
        else:
            n_psy_warn += 1
            print(f"  {YELLOW}RFAIL{RESET} {rel}")
            for line in first_error(psy_detail):
                print(f"        {DIM}{line}{RESET}")

    # Summary
    useful = n_pass + n_psy_fail + n_psy_warn
    print()
    print(f"Tested {useful} (skipped {n_sys_skip} where system STL also fails)")
    if useful == 0:
        print(
            f"{YELLOW}No comparable results — all tested files require libc++-specific features.{RESET}"
        )
        return

    pct = 100 * n_pass // useful if useful else 0
    col = GREEN if pct >= 80 else (YELLOW if pct >= 50 else RED)
    print(
        f"psychicstd: {col}{n_pass}/{useful} pass ({pct}%){RESET}  "
        f"{n_psy_fail} compile-fail  {n_psy_warn} runtime-fail"
    )


if __name__ == "__main__":
    main()
