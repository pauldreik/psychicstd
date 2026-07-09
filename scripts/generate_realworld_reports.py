#!/usr/bin/env python3
"""Regenerate every project's checked-in real-world speed report.

Runs compare_realworld_performance.py once per project known to
realworld_projects.py (see its --list) and writes each one's report to
use_on_realworld_projects/<project>_speed_report.md, matching the existing
checked-in convention -- so adding a project there is enough for it to show
up here too, with nothing to update in this script.

Usage:
  scripts/generate_realworld_reports.py [--compiler CXX]
      [--build-type {debug,release,both}] [--reps N] [--enable-ccache]
"""

import argparse
import subprocess
import sys
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
PERF_SCRIPT = REPO / "scripts" / "compare_realworld_performance.py"


def _list_projects() -> list[str]:
    out = subprocess.run(
        [sys.executable, str(PERF_SCRIPT), "--list"],
        capture_output=True,
        text=True,
        check=True,
    )
    return out.stdout.split()


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--compiler", default="c++", help="C++ compiler (default: c++)")
    ap.add_argument(
        "--build-type",
        choices=("debug", "release", "both"),
        default="both",
        help="build type(s) to report, translated per-project (default: both)",
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

    for project in _list_projects():
        output = REPO / "use_on_realworld_projects" / f"{project}_speed_report.md"
        print(f"=== {project} -> {output} ===", file=sys.stderr)
        subprocess.run(
            [
                sys.executable,
                str(PERF_SCRIPT),
                "--project",
                project,
                "--compiler",
                args.compiler,
                "--build-type",
                args.build_type,
                "--reps",
                str(args.reps),
                "--output",
                str(output),
            ]
            + (["--enable-ccache"] if args.enable_ccache else []),
            check=True,
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
