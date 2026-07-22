#!/usr/bin/env python3
"""Regenerate the real-world speed reports linked from README.md.

Runs compare_realworld_performance.py for every report linked from the project
table at the top of README.md, then updates each link's text with the generated
compilation speedup. With --build-type both, the README uses the Debug result.
Project links and hand-written comments are left unchanged.

Usage:
  scripts/generate_realworld_reports.py [--compiler CXX]
      [--build-type {debug,release,both}]
      [--reps N | --time-budget DURATION] [--max-reps N]
      [--jobs N] [--plan-only] [--enable-ccache]
"""

import argparse
import re
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
sys.path.insert(0, str(REPO / "use_on_realworld_projects"))
import realworld_projects as rw  # noqa: E402

PERF_SCRIPT = REPO / "scripts" / "compare_realworld_performance.py"
README = REPO / "README.md"
REPORT_LINK = re.compile(
    r"\[(?P<speed>[^]]+)\]\("
    r"(?P<path>use_on_realworld_projects/(?P<project>[^/)]+)_speed_report\.md)"
    r"\)"
)
COMPILE_ROW = re.compile(r"^\| compile \|.*?\|[^|]*?([0-9]+(?:\.[0-9]+)?)x(?: |\|)")


def _duration(value: str) -> float:
    match = re.fullmatch(r"([0-9]+(?:\.[0-9]+)?)([smh]?)", value)
    if not match:
        raise argparse.ArgumentTypeError("use seconds or a suffix such as 30m or 1.5h")
    seconds = (
        float(match.group(1)) * {"": 1, "s": 1, "m": 60, "h": 3600}[match.group(2)]
    )
    if seconds <= 0:
        raise argparse.ArgumentTypeError("duration must be positive")
    return seconds


def _format_duration(seconds: float) -> str:
    if seconds < 60:
        return f"{round(seconds)}s"
    minutes = round(seconds / 60)
    if minutes < 60:
        return f"{minutes}m"
    return f"{minutes // 60}h{minutes % 60:02}m"


def _repetition_cost(project: str, build_types: tuple[str, ...]) -> float:
    spec = rw.PROJECTS[project]
    try:
        # Each report repetition builds both the system and psychicstd variants.
        return 2 * sum(spec.expected_seconds[mode] for mode in build_types)
    except KeyError as error:
        raise ValueError(
            f"{project} has no expected runtime for {error.args[0]}"
        ) from error


def _budget_plan(
    projects: list[str],
    build_types: tuple[str, ...],
    budget: float,
    max_reps: int,
) -> tuple[dict[str, int], dict[str, float]]:
    costs = {project: _repetition_cost(project, build_types) for project in projects}
    minimum = sum(costs.values())
    if minimum > budget:
        raise ValueError(
            f"the budget is {_format_duration(budget)}, but one repetition of "
            f"every project is estimated to need {_format_duration(minimum)}"
        )

    reps = dict.fromkeys(projects, 1)
    remaining = budget - minimum
    while True:
        candidates = [
            project
            for project in projects
            if reps[project] < max_reps and costs[project] <= remaining
        ]
        if not candidates:
            break
        # Equalize estimated measurement time instead of repetition counts.
        project = min(candidates, key=lambda name: reps[name] * costs[name])
        reps[project] += 1
        remaining -= costs[project]
    return reps, costs


def _print_plan(
    projects: list[str], reps: dict[str, int], costs: dict[str, float]
) -> None:
    total = sum(reps[project] * costs[project] for project in projects)
    print(f"Estimated benchmark plan: {_format_duration(total)}", file=sys.stderr)
    for project in projects:
        estimate = reps[project] * costs[project]
        print(
            f"  {project:<14} {reps[project]:>2} reps  ~{_format_duration(estimate)}",
            file=sys.stderr,
        )


def _linked_reports(readme: str) -> list[tuple[str, Path]]:
    reports = []
    seen = set()
    for match in REPORT_LINK.finditer(readme):
        project = match.group("project")
        if project not in seen:
            reports.append((project, REPO / match.group("path")))
            seen.add(project)
    return reports


def _compile_speedup(report: str, build_type: str) -> str:
    in_build_type = False
    for line in report.splitlines():
        if line.startswith("### "):
            in_build_type = line == f"### {build_type.title()}"
        elif in_build_type:
            match = COMPILE_ROW.match(line)
            if match:
                return match.group(1) + "x"
    raise ValueError(f"report has no {build_type} compile speedup row")


def _update_readme(readme: str, speedups: dict[str, str]) -> str:
    def replace(match: re.Match[str]) -> str:
        project = match.group("project")
        speedup = speedups.get(project)
        if speedup is None:
            return match.group(0)
        return f"[{speedup}]({match.group('path')})"

    return REPORT_LINK.sub(replace, readme)


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
    repetitions = ap.add_mutually_exclusive_group()
    repetitions.add_argument(
        "--reps", type=int, help="fixed repetitions for every project (default: 3)"
    )
    repetitions.add_argument(
        "--time-budget",
        type=_duration,
        metavar="DURATION",
        help="allocate repetitions within an estimated total duration, e.g. 45m",
    )
    ap.add_argument(
        "--max-reps",
        type=int,
        default=5,
        help="maximum repetitions per project with --time-budget (default: 5)",
    )
    ap.add_argument(
        "--jobs",
        type=int,
        default=8,
        help="parallel build jobs passed to every benchmark (default: 8)",
    )
    ap.add_argument(
        "--plan-only",
        action="store_true",
        help="print the repetition plan without running benchmarks or writing files",
    )
    ap.add_argument(
        "--enable-ccache",
        action="store_true",
        help="leave ccache enabled (faster iteration, but skews timings -- use "
        "for debugging the script/recipes, not for real measurements)",
    )
    args = ap.parse_args()
    if args.reps is not None and args.reps <= 0:
        ap.error("--reps must be positive")
    if args.max_reps <= 0:
        ap.error("--max-reps must be positive")
    if args.jobs <= 0:
        ap.error("--jobs must be positive")

    readme = README.read_text()
    reports = _linked_reports(readme)
    known_projects = set(rw.PROJECTS)
    unknown = [project for project, _ in reports if project not in known_projects]
    if unknown:
        ap.error("README links reports for unknown projects: " + ", ".join(unknown))
    if not reports:
        ap.error("README contains no linked real-world speed reports")

    projects = [project for project, _ in reports]
    build_types = (
        ("debug", "release") if args.build_type == "both" else (args.build_type,)
    )
    costs = {project: _repetition_cost(project, build_types) for project in projects}
    if args.time_budget is not None:
        try:
            reps, costs = _budget_plan(
                projects, build_types, args.time_budget, args.max_reps
            )
        except ValueError as error:
            ap.error(str(error))
    else:
        reps = dict.fromkeys(projects, args.reps if args.reps is not None else 3)

    expected_jobs = {rw.PROJECTS[project].expected_jobs for project in projects}
    if expected_jobs != {args.jobs}:
        expected = ", ".join(str(jobs) for jobs in sorted(expected_jobs))
        print(
            f"warning: runtime estimates were measured at {expected} jobs; "
            f"this run will use {args.jobs}",
            file=sys.stderr,
        )
    _print_plan(projects, reps, costs)
    if args.plan_only:
        return 0

    speedups = {}
    readme_build_type = "debug" if args.build_type == "both" else args.build_type
    for project, output in reports:
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
                str(reps[project]),
                "--jobs",
                str(args.jobs),
                "--output",
                str(output),
            ]
            + (["--enable-ccache"] if args.enable_ccache else []),
            check=True,
        )
        speedups[project] = _compile_speedup(output.read_text(), readme_build_type)

    # Re-read after the potentially long benchmark run so unrelated edits made
    # while it was running are not overwritten.
    README.write_text(_update_readme(README.read_text(), speedups))
    print(f"=== updated compile speedups in {README} ===", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
