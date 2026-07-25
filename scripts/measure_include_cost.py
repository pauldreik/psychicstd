#!/usr/bin/env python3
"""Measure compile-time weight of each public psychicstd header.

For each public header in an include directory, this script emits a temporary
`.cpp` that contains only that header and measures compile time for:

1) system libstdc++
2) psychicstd strict mode (`_PSYCHICSTD_COMPATIBILITY_LEVEL=0`)
3) psychicstd drop-in mode (default compatibility level)

Output is markdown with per-header median times, psychich strict/libstdc++ and
psychich drop-in/libstdc++ speedup ratios, and bootstrapped 95% confidence
intervals on those ratios.
"""

from __future__ import annotations

import argparse
import math
import os
import random
import shlex
import statistics
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path

GREEN = "\U0001f7e2"
RED = "\U0001f534"
YELLOW = "\U0001f7e1"
SPINNER = "|/-\\"

BENCH_DEFAULT = int(os.environ.get("BENCH_N", "10"))
SORT_OPTIONS = (
    "header",
    "libstdc++",
    "psychic-strict",
    "psychic-dropin",
    "speedup-strict",
    "speedup-dropin",
)


def _git_toplevel() -> Path:
    out = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True,
        text=True,
        check=True,
    )
    return Path(out.stdout.strip())


REPO = _git_toplevel()


@dataclass(frozen=True)
class Row:
    header: str
    strict_ms: float | None
    strict_samples: list[float] | None
    dropin_ms: float | None
    dropin_samples: list[float] | None
    std_ms: float | None
    std_samples: list[float] | None


def bootstrap_speedup_ci(
    sys_samples: list[float],
    psy_samples: list[float],
    iters: int = 2000,
    seed: int = 12345,
) -> tuple[float, float] | None:
    if len(sys_samples) < 2 or len(psy_samples) < 2:
        return None
    rnd = random.Random(seed)
    ns, npy = len(sys_samples), len(psy_samples)
    ratios = []
    for _ in range(iters):
        s = statistics.median(rnd.choice(sys_samples) for _ in range(ns))
        p = statistics.median(rnd.choice(psy_samples) for _ in range(npy))
        if p:
            ratios.append(s / p)
    if not ratios:
        return None
    ratios.sort()
    lo = ratios[int(0.025 * (len(ratios) - 1))]
    hi = ratios[int(0.975 * (len(ratios) - 1))]
    return lo, hi


def _sort_key(raw: float | None, *, descending: bool) -> float:
    if raw is None:
        return -math.inf if descending else math.inf
    return raw


def _color(ci: tuple[float, float] | None) -> str:
    if ci is None:
        return YELLOW
    lo, hi = ci
    if lo > 1:
        return GREEN
    if hi < 1:
        return RED
    return YELLOW


def _ratio_with_ci(
    base_samples: list[float] | None,
    psy_samples: list[float] | None,
) -> str:
    if not base_samples or not psy_samples:
        return "n/a"
    base_med = statistics.median(base_samples)
    psy_med = statistics.median(psy_samples)
    if not base_med or not psy_med:
        return "n/a"
    ratio = base_med / psy_med
    ci = bootstrap_speedup_ci(base_samples, psy_samples)
    ci_txt = f" [{ci[0]:.2f}x, {ci[1]:.2f}x]" if ci else ""
    return f"{_color(ci)} {ratio:.2f}x{ci_txt}"


def _progress_bar(
    step: int,
    total_steps: int,
    header: str,
    mode: str,
    rep: int,
    reps: int,
    header_index: int,
    header_count: int,
    started: float,
) -> None:
    frac = step / total_steps if total_steps else 1.0
    elapsed = time.perf_counter() - started
    rate = elapsed / max(step, 1)
    remaining = max(total_steps - step, 0)
    eta = rate * remaining
    eta_txt = "done" if step >= total_steps else f"{eta:>5.1f}s"
    print(
        f"\r{SPINNER[step % len(SPINNER)]} "
        f"[{header_index:02d}/{header_count:02d}] {header:<22} "
        f"{mode:<13} [{rep}/{reps}] "
        f"{step:>4}/{total_steps:<4} "
        f"{frac * 100:5.1f}% ETA: {eta_txt}   ",
        end="",
        file=sys.stderr,
        flush=True,
    )


def _compile_samples(
    compiler: str,
    cpp: Path,
    output: Path,
    flags: list[str],
    reps: int,
    step: int,
    total_steps: int,
    header: str,
    mode: str,
    header_index: int,
    header_count: int,
    started: float,
) -> tuple[list[float] | None, int]:
    try:
        times = []
        for rep in range(reps):
            step += 1
            _progress_bar(
                step,
                total_steps,
                header,
                mode,
                rep + 1,
                reps,
                header_index,
                header_count,
                started,
            )
            cmd = [compiler, *flags, "-c", str(cpp), "-o", str(output)]
            start = time.perf_counter()
            subprocess.run(
                cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )
            times.append((time.perf_counter() - start) * 1000)
        return times, step
    except subprocess.CalledProcessError:
        return None, step


def _public_headers(include_dir: Path) -> list[str]:
    headers = sorted(
        path.name
        for path in include_dir.iterdir()
        if path.is_file()
        and not path.name.startswith("__psychicstd")
        and path.name != "cxxabi.h"
    )
    if not headers:
        raise RuntimeError(f"no public headers found in {include_dir}")
    return headers


def _measure(
    compiler: str,
    include_dir: Path,
    std: str,
    reps: int,
    headers: list[str] | None = None,
) -> tuple[list[Row], str]:
    cxx_flags = f"-std={std}"
    include_dir_for_flags = str(include_dir)
    strict_flags = [
        cxx_flags,
        "-nostdinc++",
        "-isystem",
        include_dir_for_flags,
        "-D_PSYCHICSTD_COMPATIBILITY_LEVEL=0",
    ]
    dropin_flags = [cxx_flags, "-nostdinc++", "-isystem", include_dir_for_flags]
    std_flags = [cxx_flags]
    if headers is None:
        headers = _public_headers(include_dir)
    total_steps = len(headers) * 3 * reps
    start = time.perf_counter()

    rows: list[Row] = []
    with tempfile.TemporaryDirectory(prefix="psychicstd-include-bench-") as td:
        td = Path(td)
        total = len(headers)
        step = 0
        for index, name in enumerate(headers, 1):
            cpp = td / f"{name}.cpp"
            cpp.write_text(f"#include <{name}>\n", encoding="utf-8")
            strict_samples, step = _compile_samples(
                compiler,
                cpp,
                td / f"{name}.strict.o",
                strict_flags,
                reps,
                step,
                total_steps,
                name,
                "psychic strict",
                index,
                total,
                start,
            )
            dropin_samples, step = _compile_samples(
                compiler,
                cpp,
                td / f"{name}.dropin.o",
                dropin_flags,
                reps,
                step,
                total_steps,
                name,
                "psychic drop-in",
                index,
                total,
                start,
            )
            std_samples, step = _compile_samples(
                compiler,
                cpp,
                td / f"{name}.std.o",
                std_flags,
                reps,
                step,
                total_steps,
                name,
                "libstdc++",
                index,
                total,
                start,
            )
            print(
                f"\r[{index}/{total}] done {name}{' ' * 16}",
                file=sys.stderr,
                flush=True,
            )

            strict_ms = statistics.median(strict_samples) if strict_samples else None
            dropin_ms = statistics.median(dropin_samples) if dropin_samples else None
            std_ms = statistics.median(std_samples) if std_samples else None

            rows.append(
                Row(
                    header=name,
                    strict_ms=strict_ms,
                    strict_samples=strict_samples,
                    dropin_ms=dropin_ms,
                    dropin_samples=dropin_samples,
                    std_ms=std_ms,
                    std_samples=std_samples,
                )
            )

    compiler_version = ""
    try:
        out = subprocess.run(
            [compiler, "--version"], capture_output=True, text=True, check=True
        )
        compiler_version = out.stdout.splitlines()[0].strip()
    except (OSError, subprocess.CalledProcessError):
        compiler_version = compiler
    return rows, compiler_version


def _render(rows: list[Row], sort_by: str, ascending: bool) -> tuple[str, str]:
    descending = not ascending
    if sort_by == "header":
        rows = sorted(rows, key=lambda r: r.header, reverse=False)
    elif sort_by == "libstdc++":
        rows = sorted(
            rows,
            key=lambda r: _sort_key(r.std_ms, descending=descending),
            reverse=descending,
        )
    elif sort_by == "psychic-strict":
        rows = sorted(
            rows,
            key=lambda r: _sort_key(r.strict_ms, descending=descending),
            reverse=descending,
        )
    elif sort_by == "psychic-dropin":
        rows = sorted(
            rows,
            key=lambda r: _sort_key(r.dropin_ms, descending=descending),
            reverse=descending,
        )
    elif sort_by == "speedup-strict":

        def _metric(row: Row) -> float | None:
            return (
                statistics.median(row.std_samples)
                / statistics.median(row.strict_samples)
                if row.std_samples and row.strict_samples
                else None
            )

        rows = sorted(
            rows,
            key=lambda r: _sort_key(_metric(r), descending=descending),
            reverse=descending,
        )
    else:

        def _metric(row: Row) -> float | None:
            return (
                statistics.median(row.std_samples)
                / statistics.median(row.dropin_samples)
                if row.std_samples and row.dropin_samples
                else None
            )

        rows = sorted(
            rows,
            key=lambda r: _sort_key(_metric(r), descending=descending),
            reverse=descending,
        )

    lines = [
        "| header | psychic strict (ms) | psychic drop-in (ms) | libstdc++ (ms) | strict speedup | drop-in speedup |",
        "| --- | ---: | ---: | ---: | ---: | ---: |",
    ]

    for row in rows:
        strict_cell = f"{row.strict_ms:.1f}" if row.strict_ms is not None else "n/a"
        dropin_cell = f"{row.dropin_ms:.1f}" if row.dropin_ms is not None else "n/a"
        std_cell = f"{row.std_ms:.1f}" if row.std_ms is not None else "n/a"

        strict_ratio = _ratio_with_ci(row.std_samples, row.strict_samples)
        dropin_ratio = _ratio_with_ci(row.std_samples, row.dropin_samples)
        lines.append(
            f"| {row.header} | {strict_cell} | {dropin_cell} | {std_cell} | "
            f"{strict_ratio} | {dropin_ratio} |"
        )
    return "\n".join(lines)


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument(
        "--compiler",
        default="g++",
        help="C++ compiler for all measurements (default: g++)",
    )
    ap.add_argument(
        "--include",
        type=Path,
        default=REPO / "include",
        help="Directory with public psychicstd headers (default: include/)",
    )
    ap.add_argument(
        "--header",
        action="append",
        dest="headers",
        metavar="NAME",
        help="Measure only this public header; may be repeated",
    )
    ap.add_argument(
        "-n",
        "--reps",
        type=int,
        default=BENCH_DEFAULT,
        help=f"compilations per header (default: {BENCH_DEFAULT})",
    )
    ap.add_argument(
        "--std",
        default="c++20",
        help="C++ language mode (default: c++20)",
    )
    ap.add_argument(
        "--sort",
        choices=SORT_OPTIONS,
        default="libstdc++",
        help="Sort key for output table (default: libstdc++).",
    )
    ap.add_argument(
        "--ascending",
        action="store_true",
        help="Sort ascending instead of descending",
    )
    ap.add_argument(
        "--output",
        type=Path,
        help="Write markdown output to this file as well as stdout",
    )
    args = ap.parse_args()

    if args.reps < 1:
        ap.error("--reps must be at least 1")

    include_dir = args.include.resolve()
    if not include_dir.is_dir():
        ap.error(f"--include is not a directory: {include_dir}")

    available_headers = _public_headers(include_dir)
    headers = None
    if args.headers:
        unknown = sorted(set(args.headers) - set(available_headers))
        if unknown:
            ap.error(f"unknown public header(s): {', '.join(unknown)}")
        headers = sorted(set(args.headers))

    os.environ["CCACHE_DISABLE"] = "1"

    rows, version = _measure(
        args.compiler, include_dir, args.std, args.reps, headers=headers
    )
    table = _render(rows, args.sort, args.ascending)

    reproduce = [
        "scripts/measure_include_cost.py",
        f"--compiler={shlex.quote(args.compiler)}",
        f"--reps={args.reps}",
        f"--std={shlex.quote(args.std)}",
        f"--sort={shlex.quote(args.sort)}",
    ]
    if args.ascending:
        reproduce.append("--ascending")
    if args.include.resolve() != (REPO / "include").resolve():
        reproduce.append(f"--include={shlex.quote(str(include_dir))}")
    for header in headers or []:
        reproduce.append(f"--header={shlex.quote(header)}")
    reproduce_line = " ".join(reproduce)
    if args.output is not None:
        reproduce_line += f" --output={shlex.quote(str(args.output))}"

    report = [
        "# Include cost report\n",
        (
            "Measures compile-time cost for each public psychicstd header. It generates "
            "a temporary cpp file for each header with a single `#include` and measures "
            "compilation time. That is repeated for three modes: `psychic strict`, "
            "`psychic drop-in`, and `libstdc++`.\n"
        ),
        f"Compiler: `{version}`\n",
        f"Standard: `{args.std}`\n",
        f"Repetitions per header: `{args.reps}`\n",
        f"Headers: `{', '.join(headers)}`\n" if headers else "",
        "",
        (
            f"{GREEN} whole CI above 1x (reliably faster) · "
            f"{RED} whole CI below 1x (reliably slower) · "
            f"{YELLOW} CI overlaps 1x (not distinguishable)."
        ),
        f"Sorted by: `{args.sort}` ({'ascending' if args.ascending else 'descending'}).",
        "",
        table,
        "",
        "---",
        "Reproduce on your machine:",
        f"python3 {reproduce_line}",
        "",
    ]
    text = "\n".join(report)
    print(text)

    if args.output:
        args.output.write_text(text + "\n", encoding="utf-8")
        if (
            subprocess.run(
                ["which", "mdformat"], capture_output=True, check=False
            ).returncode
            == 0
        ):
            subprocess.run(["mdformat", str(args.output)], check=True)
        print(f"Report written to {args.output}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
