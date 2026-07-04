#!/usr/bin/env python3
"""Compare psychicstd compile-time performance: the current working tree vs a
git ref, on THIS machine, and print a markdown diff (suitable for a PR comment).

It benchmarks the same set of files twice -- once with the reference's headers,
once with the working tree's -- swapping only the psychicstd include dir. Running
both sides on one machine cancels out absolute host speed, so the comparison is
meaningful even though CI runners vary. This is exactly what the perf-pr CI job
runs, so the numbers are reproducible.

Usage:
  scripts/compare_performance.py [compiler] [ref]

  compiler   C++ compiler to benchmark with   (default: g++)
  ref        git ref to compare against       (default: origin/main)

Env:
  BENCH_N    reps per file (median); inherited by run_bench.py (CI uses 5)

The markdown diff is printed to stdout; all progress goes to stderr. Nothing is
left behind (the temporary worktree and result files are cleaned up).
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def sh(cmd, **kw):
    return subprocess.run(cmd, check=True, **kw)


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Compare compile-time performance of the working tree vs a git ref.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="examples:\n"
        "  scripts/compare_performance.py                 # g++ vs origin/main\n"
        "  scripts/compare_performance.py g++ origin/main\n"
        "  scripts/compare_performance.py clang++-21 HEAD~1\n"
        "  BENCH_N=10 scripts/compare_performance.py      # more reps, tighter CIs\n",
    )
    ap.add_argument(
        "compiler", nargs="?", default="g++", help="C++ compiler (default: g++)"
    )
    ap.add_argument(
        "ref",
        nargs="?",
        default="origin/main",
        help="git ref to compare against (default: origin/main)",
    )
    ap.add_argument(
        "-n",
        "--reps",
        type=int,
        default=None,
        help="reps per file (median): more = tighter confidence intervals but "
        "slower. Default: run_bench's default (10), or $BENCH_N if set.",
    )
    args = ap.parse_args()

    # run_bench.py reads the rep count from $BENCH_N; --reps sets it for both runs.
    child_env = os.environ.copy()
    if args.reps is not None:
        child_env["BENCH_N"] = str(args.reps)

    repo = Path(
        sh(
            ["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True
        ).stdout.strip()
    )
    run_bench = repo / "benchmarks" / "compile_time" / "run_bench.py"
    bench_diff = repo / "tools" / "bench_diff.py"

    tmp = Path(tempfile.mkdtemp(prefix="psychicstd-perf-"))
    worktree = tmp / "ref"
    base_json = tmp / "base.json"
    head_json = tmp / "head.json"

    try:
        # A worktree at the reference gives us its include/ to compare against.
        print(f"Checking out {args.ref} into a temporary worktree ...", file=sys.stderr)
        sh(["git", "worktree", "add", "--quiet", "--detach", str(worktree), args.ref])

        # rapidjson is used as a real-code benchmark input; a configure fetches it.
        subprocess.run(
            ["cmake", "-B", "build"],
            cwd=repo,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        rapidjson = repo / "build" / "_deps" / "rapidjson-src" / "include"
        extra = (
            ["--extra-include", f"rapidjson:{rapidjson}"] if rapidjson.is_dir() else []
        )

        def bench(include_dir, out, label):
            print(f"Benchmarking {label} ...", file=sys.stderr)
            # run_bench progress goes to our stderr; only the diff reaches stdout.
            sh(
                [
                    sys.executable,
                    str(run_bench),
                    args.compiler,
                    str(include_dir),
                    "--json",
                    str(out),
                    *extra,
                ],
                cwd=repo,
                stdout=sys.stderr,
                env=child_env,
            )

        bench(worktree / "include", base_json, f"{args.ref} (reference)")
        bench(repo / "include", head_json, "working tree")

        sh(
            [
                sys.executable,
                str(bench_diff),
                "--base",
                str(base_json),
                "--head",
                str(head_json),
                "--reproduce",
                "scripts/compare_performance.py "
                + (f"--reps {args.reps} " if args.reps is not None else "")
                + f"{args.compiler} {args.ref}",
            ],
            cwd=repo,
        )
    finally:
        subprocess.run(
            ["git", "worktree", "remove", "--force", str(worktree)],
            cwd=repo,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        shutil.rmtree(tmp, ignore_errors=True)

    return 0


if __name__ == "__main__":
    sys.exit(main())
