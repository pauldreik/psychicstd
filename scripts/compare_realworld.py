#!/usr/bin/env python3
"""Compare a real-world project's build time (configure / compile / tests) with
psychicstd: the working tree vs a git ref, on THIS machine, and print a markdown
diff (suitable for a PR comment).

It builds the project three ways on one host -- with libstdc++ (a noise proxy),
with the reference's psychicstd headers, and with the working tree's -- swapping
only the include dir. Running both sides on one runner cancels absolute host
speed, so the comparison is meaningful even though CI runners vary. The markdown
is produced by tools/bench_diff.py, so it looks exactly like the compile-time
perf diff, but per phase.

Usage:
  scripts/compare_realworld.py [project] [--ref REF] [--compiler CXX] [--reps N]
"""

import argparse
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(
    subprocess.run(
        ["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True
    ).stdout.strip()
)
sys.path.insert(0, str(REPO / "use_on_realworld_projects"))
import realworld_projects as rw  # noqa: E402

# psychicstd's link additions (GCC 12-compatible: drop libstdc++, spell out the rest).
PSY_LDFLAGS = "-nodefaultlibs"
PSY_LIBS = "-lsupc++ -lm -lc -lgcc_s -lgcc"


def compiler_version(cxx):
    try:
        out = subprocess.run(
            [cxx, "--version"], capture_output=True, text=True, check=True
        ).stdout
        return out.splitlines()[0].strip()
    except Exception:
        return None


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument(
        "project",
        nargs="?",
        default="rdfind",
        choices=sorted(rw.PROJECTS),
        help="project to benchmark (default: rdfind)",
    )
    ap.add_argument("--ref", default="origin/main", help="git ref to compare against")
    ap.add_argument("--compiler", default="c++", help="C++ compiler (default: c++)")
    ap.add_argument(
        "--reps", type=int, default=3, help="build repetitions (default: 3)"
    )
    args = ap.parse_args()

    build = rw.PROJECTS[args.project]

    def sys_tc():
        return rw.Toolchain(args.compiler, "-std=c++20")

    def psy_tc(include):
        return rw.Toolchain(
            args.compiler,
            f"-std=c++20 -nostdinc++ -isystem {include}",
            PSY_LDFLAGS,
            PSY_LIBS,
        )

    tmp = Path(tempfile.mkdtemp(prefix="psychicstd-rw-"))
    worktree = tmp / "ref"
    try:
        print(f"Checking out {args.ref} into a temporary worktree ...", file=sys.stderr)
        subprocess.run(
            ["git", "worktree", "add", "--quiet", "--detach", str(worktree), args.ref],
            check=True,
        )

        # Measure system twice (base/head) so its drift is a real noise proxy.
        variants = {
            "system_base": sys_tc(),
            "ref": psy_tc(worktree / "include"),
            "system_head": sys_tc(),
            "head": psy_tc(REPO / "include"),
        }
        samples = {v: {p: [] for p in rw.PHASES} for v in variants}
        for rep in range(args.reps):
            for name, tc in variants.items():
                print(
                    f"[{rep + 1}/{args.reps}] building {args.project}: {name}",
                    file=sys.stderr,
                )
                t = build(tc)
                for p in rw.PHASES:
                    samples[name][p].append(t[p])

        def side(sys_key, psy_key, cxx_ver):
            d = {
                p: {
                    "system_samples": samples[sys_key][p],
                    "psychicstd_samples": samples[psy_key][p],
                }
                for p in rw.PHASES
            }
            d["__meta__"] = {"compiler_version": cxx_ver}
            return d

        ver = compiler_version(args.compiler)
        base_json = tmp / "base.json"
        head_json = tmp / "head.json"
        base_json.write_text(json.dumps(side("system_base", "ref", ver)))
        head_json.write_text(json.dumps(side("system_head", "head", ver)))

        subprocess.run(
            [
                sys.executable,
                str(REPO / "tools" / "bench_diff.py"),
                "--base",
                str(base_json),
                "--head",
                str(head_json),
                "--title",
                f"{args.project} build-time diff (configure / compile / tests)",
                "--what",
                f"{args.project} build time with psychicstd",
                "--reproduce",
                f"scripts/compare_realworld.py {args.project} --reps {args.reps}",
            ],
            check=True,
        )
    finally:
        subprocess.run(
            ["git", "worktree", "remove", "--force", str(worktree)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        shutil.rmtree(tmp, ignore_errors=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
