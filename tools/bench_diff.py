#!/usr/bin/env python3
"""
Compares two compile-time benchmark result files (from run_bench.py --json) and
emits a markdown summary for a PR comment.

Both files must be produced in the SAME CI job on the SAME host -- one with the
PR branch's headers, one with main's -- because absolute compile times vary
wildly between runners. We compare the psychicstd compile time per benchmark
file (main vs PR).

Each result carries the raw per-rep samples, so we put a 95% confidence
interval on the percentage change by bootstrapping the difference of medians.
Colors: green = faster, red = slower, yellow = within noise -- a change is only
colored green/red when its CI excludes 0 (distinguishable from noise) and it
clears the practical floors; everything else is yellow. The system (libstdc++)
time is identical work on both runs, so its drift is a global noise indicator.

Usage:
  python3 tools/bench_diff.py --base base.json --head head.json [--threshold 5]
"""

import argparse
import json
import random
import statistics
from pathlib import Path


def load(path: str) -> dict:
    return json.loads(Path(path).read_text())


def bench_names(cache: dict) -> set:
    """Benchmark names in a result file, excluding reserved metadata keys
    (e.g. __meta__)."""
    return {k for k in cache if not k.startswith("__")}


def samples(cache: dict, name: str, key: str) -> list[float] | None:
    """Per-rep samples for a metric; fall back to a single-point list if an
    older result file only stored the median."""
    entry = cache.get(name, {})
    s = entry.get(f"{key}_samples")
    if isinstance(s, list) and s:
        return [float(x) for x in s]
    v = entry.get(f"{key}_ms")
    return [float(v)] if isinstance(v, (int, float)) else None


def bootstrap_delta_ci(base_s, head_s, iters=2000, seed=12345):
    """95% CI for the percentage change of the median, by bootstrap resampling.
    Returns (lo, hi) or None when there aren't enough samples to resample."""
    if len(base_s) < 2 or len(head_s) < 2:
        return None
    rnd = random.Random(seed)
    nb, nh = len(base_s), len(head_s)
    deltas = []
    for _ in range(iters):
        b = statistics.median([rnd.choice(base_s) for _ in range(nb)])
        h = statistics.median([rnd.choice(head_s) for _ in range(nh)])
        if b:
            deltas.append((h - b) / b * 100.0)
    if not deltas:
        return None
    deltas.sort()
    lo = deltas[int(0.025 * (len(deltas) - 1))]
    hi = deltas[int(0.975 * (len(deltas) - 1))]
    return lo, hi


def main() -> None:
    ap = argparse.ArgumentParser(
        description=(
            "Compare two compile-time benchmark runs and print a markdown summary "
            "to stdout (suitable for a PR comment).\n\n"
            "Each input is a JSON file you generate with `run_bench.py --json`: run "
            "it once per side (typically main vs your branch), choosing the output "
            "filenames yourself. This tool writes nothing to disk -- it prints the "
            "diff to stdout; redirect it (`> diff.md`) if you want to keep it."
        ),
        epilog=(
            "example:\n"
            "  # 1. produce the two inputs (same machine; swap only the include dir)\n"
            "  python3 benchmarks/compile_time/run_bench.py g++ /path/to/main/include --json base.json\n"
            "  python3 benchmarks/compile_time/run_bench.py g++ include                --json head.json\n"
            "  # 2. compare them; markdown goes to stdout\n"
            "  python3 tools/bench_diff.py --base base.json --head head.json > diff.md\n"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument(
        "--base",
        required=True,
        metavar="JSON",
        help="baseline results file from `run_bench.py --json` (e.g. main)",
    )
    ap.add_argument(
        "--head",
        required=True,
        metavar="JSON",
        help="new results file from `run_bench.py --json` (e.g. your branch)",
    )
    ap.add_argument(
        "--threshold",
        type=float,
        default=5.0,
        help="Percent change in psychicstd compile time to flag (default: 5)",
    )
    ap.add_argument(
        "--min-abs-ms",
        type=float,
        default=3.0,
        help="Also require this absolute ms change to flag, so fast headers near "
        "the fixed compiler-startup floor don't trip on noise (default: 3)",
    )
    ap.add_argument(
        "--reproduce",
        metavar="CMD",
        help="Append a line telling the reader how to regenerate this diff "
        "locally (e.g. the compare_performance.sh invocation)",
    )
    args = ap.parse_args()

    base = load(args.base)
    head = load(args.head)
    thr, min_abs = args.threshold, args.min_abs_ms

    GREEN, RED, YELLOW = "\U0001f7e2", "\U0001f534", "\U0001f7e1"

    def crosses_zero(ci):
        return ci is None or ci[0] < 0 < ci[1]

    def ci_str(ci):
        return "" if ci is None else f" [{ci[0]:+.1f}, {ci[1]:+.1f}]"

    common = sorted(bench_names(base) & bench_names(head))
    # (name, base_ms, head_ms, delta_pct, ci, color)
    rows: list[tuple] = []
    for name in common:
        b_s = samples(base, name, "psychicstd")
        h_s = samples(head, name, "psychicstd")
        if not b_s or not h_s:
            continue
        b, h = statistics.median(b_s), statistics.median(h_s)
        if b == 0:
            continue
        delta = (h - b) / b * 100.0
        ci = bootstrap_delta_ci(b_s, h_s)
        # A change is real only if it stands out from the run-to-run noise (its
        # CI excludes 0) and clears the practical floors. Otherwise it's noise.
        real = not crosses_zero(ci) and abs(delta) >= thr and abs(h - b) >= min_abs
        color = (RED if delta > 0 else GREEN) if real else YELLOW
        rows.append((name, b, h, delta, ci, color))

    # System-time drift on the same files: a proxy for host/run noise, since the
    # system config compiles identical work on both runs.
    sys_drift = []
    for name in common:
        b_s = samples(base, name, "system")
        h_s = samples(head, name, "system")
        if b_s and h_s:
            b, h = statistics.median(b_s), statistics.median(h_s)
            if b:
                sys_drift.append(abs(h - b) / b * 100.0)
    noise = statistics.median(sys_drift) if sys_drift else 0.0

    slower = sorted([r for r in rows if r[5] == RED], key=lambda r: -r[3])
    faster = sorted([r for r in rows if r[5] == GREEN], key=lambda r: r[3])
    noisy = sorted([r for r in rows if r[5] == YELLOW], key=lambda r: r[0])

    head_ver = head.get("__meta__", {}).get("compiler_version")
    base_ver = base.get("__meta__", {}).get("compiler_version")
    compiler = head_ver or base_ver or "unknown"
    if base_ver and head_ver and base_ver != head_ver:
        compiler = f"PR: {head_ver} / main: {base_ver}"

    lines: list[str] = ["## Compile-time performance diff\n"]
    lines.append(f"Compiler: `{compiler}`.\n")
    lines.append(
        f"psychicstd compile time, main vs this PR (same runner). "
        f"{GREEN} faster · {RED} slower · {YELLOW} within noise. A change is colored only "
        f"when its bootstrap 95% CI excludes 0 and it clears ±{thr:g}% / {min_abs:g}ms. "
        f"Median system-time drift (noise proxy): **{noise:.1f}%**.\n"
    )

    def table(rowset):
        out = [
            "| benchmark | main | PR | Δ (95% CI) |",
            "|-----------|-----:|---:|:--|",
        ]
        for name, b, h, delta, ci, color in rowset:
            out.append(
                f"| `{name}` | {b:.1f}ms | {h:.1f}ms | {color} {delta:+.1f}%{ci_str(ci)} |"
            )
        return out

    if slower or faster:
        parts = []
        if slower:
            parts.append(f"{RED} {len(slower)} slower")
        if faster:
            parts.append(f"{GREEN} {len(faster)} faster")
        parts.append(f"{YELLOW} {len(noisy)} within noise")
        lines.append(" · ".join(parts) + ".\n")
        lines.extend(table(slower + faster))
        lines.append("")
    else:
        lines.append(f"No significant changes ({YELLOW} {len(noisy)} within noise).\n")

    if noisy:
        lines.append(
            f"<details><summary>{YELLOW} {len(noisy)} benchmark(s) within noise</summary>\n"
        )
        lines.extend(table(noisy))
        lines.append("\n</details>")

    only_head = sorted(bench_names(head) - bench_names(base))
    only_base = sorted(bench_names(base) - bench_names(head))
    if only_head or only_base:
        notes = []
        if only_head:
            notes.append(f"new on this PR: {', '.join(f'`{n}`' for n in only_head)}")
        if only_base:
            notes.append(f"only on main: {', '.join(f'`{n}`' for n in only_base)}")
        lines.append("\n" + " · ".join(notes))

    if args.reproduce:
        lines.append(f"\n---\nReproduce this on your machine: `{args.reproduce}`")

    print("\n".join(lines))


if __name__ == "__main__":
    main()
