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
A flagged change whose CI crosses 0 is marked low-confidence (likely noise --
raise BENCH_N or ignore). The system (libstdc++) time is identical work on both
runs, so its drift is reported as a global noise indicator.

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
    ap = argparse.ArgumentParser(description="Diff two compile-time benchmark runs")
    ap.add_argument("--base", required=True, metavar="JSON", help="Base (main) results")
    ap.add_argument("--head", required=True, metavar="JSON", help="Head (PR) results")
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
    args = ap.parse_args()

    base = load(args.base)
    head = load(args.head)
    thr, min_abs = args.threshold, args.min_abs_ms

    common = sorted(set(base) & set(head))
    # (name, base_ms, head_ms, delta_pct, ci_or_None)
    changed: list[tuple] = []
    unchanged: list[tuple] = []
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
        row = (name, b, h, delta, ci)
        significant = abs(delta) >= thr and abs(h - b) >= min_abs
        (changed if significant else unchanged).append(row)

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

    changed.sort(key=lambda r: -r[3])  # worst regression first
    regressions = [r for r in changed if r[3] > 0]
    improvements = [r for r in changed if r[3] < 0]

    def ci_str(ci):
        if ci is None:
            return ""
        lo, hi = ci
        return f" [{lo:+.1f}, {hi:+.1f}]"

    def crosses_zero(ci):
        return ci is not None and ci[0] < 0 < ci[1]

    lines: list[str] = ["## Compile-time performance diff\n"]
    lines.append(
        f"psychicstd compile time, main vs this PR (same runner; flagging changes "
        f"≥{thr:g}% and ≥{min_abs:g}ms). Δ shows the median change with a bootstrap "
        f"95% CI. Median system-time drift (noise proxy): **{noise:.1f}%**.\n"
    )

    if not changed:
        lines.append(f"No changes beyond ±{thr:g}% / {min_abs:g}ms.\n")
    else:
        parts = []
        if regressions:
            parts.append(f"\U0001f534 {len(regressions)} slower")
        if improvements:
            parts.append(f"\U0001f7e2 {len(improvements)} faster")
        lines.append(", ".join(parts) + ".\n")
        lines.append("| benchmark | main | PR | Δ (95% CI) |")
        lines.append("|-----------|-----:|---:|:--|")
        for name, b, h, delta, ci in changed:
            emoji = "\U0001f534" if delta > 0 else "\U0001f7e2"
            sign = f"+{delta:.1f}" if delta > 0 else f"{delta:.1f}"
            warn = " ⚠️ low-confidence" if crosses_zero(ci) else ""
            lines.append(
                f"| `{name}` | {b:.1f}ms | {h:.1f}ms | {emoji} {sign}%{ci_str(ci)}{warn} |"
            )
        lines.append("")

    if unchanged:
        lines.append(
            f"<details><summary>{len(unchanged)} benchmark(s) within noise</summary>\n"
        )
        lines.append("| benchmark | main | PR | Δ (95% CI) |")
        lines.append("|-----------|-----:|---:|:--|")
        for name, b, h, delta, ci in sorted(unchanged, key=lambda r: r[0]):
            lines.append(
                f"| `{name}` | {b:.1f}ms | {h:.1f}ms | {delta:+.1f}%{ci_str(ci)} |"
            )
        lines.append("\n</details>")

    only_head = sorted(set(head) - set(base))
    only_base = sorted(set(base) - set(head))
    if only_head or only_base:
        notes = []
        if only_head:
            notes.append(f"new on this PR: {', '.join(f'`{n}`' for n in only_head)}")
        if only_base:
            notes.append(f"only on main: {', '.join(f'`{n}`' for n in only_base)}")
        lines.append("\n" + " · ".join(notes))

    print("\n".join(lines))


if __name__ == "__main__":
    main()
