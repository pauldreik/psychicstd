#!/usr/bin/env python3
"""Compare a real-world project's build time (per its own Project.phases, e.g.
configure / compile / run tests) with psychicstd: the working tree vs a git
ref, on THIS machine, and print a markdown diff (suitable for a PR comment).

It builds the project three ways on one host -- with libstdc++ (a noise proxy),
with the reference's psychicstd headers, and with the working tree's -- swapping
only the include dir. Running both sides on one runner cancels absolute host
speed, so the comparison is meaningful even though CI runners vary. The markdown
is produced by tools/bench_diff.py, so it looks exactly like the compile-time
perf diff, but per phase.

Usage:
  scripts/compare_realworld.py [project] [--ref REF] [--compiler CXX]
      [--build-type {debug,release}] [--reps N]
"""

import argparse
import json
import subprocess
import sys
import tempfile
from collections.abc import Callable
from pathlib import Path


def _git_toplevel() -> Path:
    out = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True,
        text=True,
        check=True,
    )
    return Path(out.stdout.strip())


REPO = _git_toplevel()
sys.path.insert(0, str(REPO / "use_on_realworld_projects"))
import realworld_projects as rw  # noqa: E402

# psychicstd's link additions (GCC 12-compatible: drop libstdc++, spell out the rest).
PSY_LDFLAGS = "-nodefaultlibs"
PSY_LIBS = "-lsupc++ -lm -lc -lgcc_s -lgcc"

# Selectable from the outside; each project's recipe translates this into its own
# build system's convention (CMAKE_BUILD_TYPE, an -O flag, ...) -- see Toolchain.
BUILD_TYPES = ("debug", "release")


def compiler_version(cxx: str) -> str | None:
    try:
        out = subprocess.run(
            [cxx, "--version"], capture_output=True, text=True, check=True
        ).stdout
        return out.splitlines()[0].strip()
    except (OSError, subprocess.CalledProcessError, IndexError):
        return None


def sys_tc(compiler: str, build_type: str, enable_ccache: bool = False) -> rw.Toolchain:
    return rw.Toolchain(
        compiler, "-std=c++20", build_type=build_type, enable_ccache=enable_ccache
    )


def psy_tc(
    compiler: str, include: Path, build_type: str, enable_ccache: bool = False
) -> rw.Toolchain:
    return rw.Toolchain(
        compiler,
        f"-std=c++20 -nostdinc++ -isystem {include}",
        PSY_LDFLAGS,
        PSY_LIBS,
        build_type=build_type,
        enable_ccache=enable_ccache,
    )


def _build_matrix(
    variants: dict[str, rw.Toolchain],
    build: Callable[[rw.Toolchain], dict[str, float]],
    reps: int,
    project: str,
) -> dict[str, dict[str, list[float]]]:
    phases = rw.PROJECTS[project].phases
    samples: dict[str, dict[str, list[float]]] = {
        name: {p: [] for p in phases} for name in variants
    }
    for rep in range(reps):
        for name, tc in variants.items():
            print(f"[{rep + 1}/{reps}] building {project}: {name}", file=sys.stderr)
            t = build(tc)
            for p in phases:
                samples[name][p].append(t[p])
    return samples


def measure_project(
    project: str,
    compiler: str,
    build_type: str,
    reps: int,
    include: Path,
    enable_ccache: bool = False,
) -> dict[str, dict[str, list[float]]]:
    """Build `project` `reps` times under the system toolchain and psychicstd
    (headers from `include`); return {"system"|"psychicstd": {phase: [ms, ...]}}.

    Unlike main()'s 4-way matrix, this only measures one set of psychicstd
    headers -- there's no git ref involved, so it's reusable for reporting the
    working tree's absolute speedup rather than a main-vs-PR regression diff.
    """
    variants = {
        "system": sys_tc(compiler, build_type, enable_ccache),
        "psychicstd": psy_tc(compiler, include, build_type, enable_ccache),
    }
    return _build_matrix(variants, rw.PROJECTS[project].build, reps, project)


def _side(
    samples: dict[str, dict[str, list[float]]],
    sys_key: str,
    psy_key: str,
    cxx_ver: str | None,
    phases: tuple[str, ...],
) -> dict:
    d = {
        p: {
            "system_samples": samples[sys_key][p],
            "psychicstd_samples": samples[psy_key][p],
        }
        for p in phases
    }
    d["__meta__"] = {"compiler_version": cxx_ver}
    return d


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
        "--build-type",
        choices=BUILD_TYPES,
        default="debug",
        help="build type, translated per-project (default: debug)",
    )
    ap.add_argument(
        "--reps", type=int, default=3, help="build repetitions (default: 3)"
    )
    ap.add_argument(
        "--enable-ccache",
        action="store_true",
        help="leave ccache enabled (faster iteration, but skews timings -- use "
        "for debugging the script/recipes, not for real measurements)",
    )
    args = ap.parse_args()

    build = rw.PROJECTS[args.project].build

    with tempfile.TemporaryDirectory(
        prefix="psychicstd-rw-", ignore_cleanup_errors=True
    ) as tmp_dir:
        tmp = Path(tmp_dir)
        worktree = tmp / "ref"
        print(f"Checking out {args.ref} into a temporary worktree ...", file=sys.stderr)
        subprocess.run(
            ["git", "worktree", "add", "--quiet", "--detach", str(worktree), args.ref],
            check=True,
        )
        try:
            # Measure system twice (base/head) so its drift is a real noise proxy.
            variants = {
                "system_base": sys_tc(
                    args.compiler, args.build_type, args.enable_ccache
                ),
                "ref": psy_tc(
                    args.compiler,
                    worktree / "include",
                    args.build_type,
                    args.enable_ccache,
                ),
                "system_head": sys_tc(
                    args.compiler, args.build_type, args.enable_ccache
                ),
                "head": psy_tc(
                    args.compiler, REPO / "include", args.build_type, args.enable_ccache
                ),
            }
            samples = _build_matrix(variants, build, args.reps, args.project)
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(worktree)],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=False,
            )

        phases = rw.PROJECTS[args.project].phases
        ver = compiler_version(args.compiler)
        base_json = tmp / "base.json"
        head_json = tmp / "head.json"
        base_json.write_text(
            json.dumps(_side(samples, "system_base", "ref", ver, phases))
        )
        head_json.write_text(
            json.dumps(_side(samples, "system_head", "head", ver, phases))
        )

        subprocess.run(
            [
                sys.executable,
                str(REPO / "tools" / "bench_diff.py"),
                "--base",
                str(base_json),
                "--head",
                str(head_json),
                "--title",
                f"{args.project} build-time diff ({' / '.join(phases)})",
                "--what",
                f"{args.project} build time with psychicstd",
                "--reproduce",
                f"scripts/compare_realworld.py {args.project} "
                f"--build-type {args.build_type} --reps {args.reps}"
                + (" --enable-ccache" if args.enable_ccache else ""),
            ],
            check=True,
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
