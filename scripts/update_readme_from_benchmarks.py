#!/usr/bin/env python3
"""Sync selected inline README benchmark snippets from generated reports."""

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
README = REPO_ROOT / "README.md"
COMPILE_TIME_REPORT = REPO_ROOT / "compile_time.md"
STARTUP_REPORT = REPO_ROOT / "startup.md"
REALWORLD_REPORT_LINK = re.compile(
    r"\[(?P<speed>[^]]+)\]\("
    r"(?P<path>use_on_realworld_projects/(?P<project>[^/)]+)_speed_report\.md)"
    r"\)"
)
COMPILE_SPEEDUP_ROW = re.compile(
    r"^\| compile \|.*?\|[^|]*?([0-9]+(?:\.[0-9]+)?)x(?: |\|)"
)


def _replace_block(readme: str, marker: str, replacement: str) -> str:
    match = _find_marker_match(readme, marker)
    replacement = replacement.strip("\n")
    return (
        readme[: match.start("start")]
        + match.group("start")
        + "\n"
        + replacement
        + "\n"
        + match.group("end")
        + readme[match.end("end") :]
    )


def _coerce_marker_body_lines(body: str) -> list[str]:
    lines = body.splitlines()
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    return lines


def _extract_realworld_speedup(report: str, build_types: tuple[str, ...]) -> str:
    for build_type in build_types:
        in_build_type = False
        for line in report.splitlines():
            if line.startswith("### "):
                in_build_type = line == f"### {build_type.title()}"
            elif in_build_type:
                match = COMPILE_SPEEDUP_ROW.match(line)
                if match:
                    return match.group(1) + "x"
    raise ValueError(
        "no matching build-type compile speedup row in report; "
        f"expected one of: {', '.join(build_types)}"
    )


def _replace_realworld_rows(readme: str, marker: str) -> str:
    match = _find_marker_match(readme, marker)
    lines = _coerce_marker_body_lines(match.group("body"))
    updated: list[str] = []
    changed = 0

    for line in lines:
        row = line
        report_match = REALWORLD_REPORT_LINK.search(line)
        if report_match:
            report_path = REPO_ROOT / report_match.group("path")
            if not report_path.is_file():
                raise RuntimeError(
                    f"README marker {marker}: missing report {report_match.group('path')}"
                )
            speedup = _extract_realworld_speedup(
                report_path.read_text(),
                ("debug", "release"),
            )
            row = (
                line[: report_match.start()]
                + f"[{speedup}]({report_match.group('path')})"
                + line[report_match.end() :]
            )
            changed += 1
        updated.append(row)

    if changed == 0:
        raise RuntimeError(
            f"README marker {marker}: no real-world report rows found for update"
        )

    updated_body = "\n".join(updated)
    return (
        readme[: match.start("start")]
        + match.group("start")
        + "\n"
        + updated_body
        + "\n"
        + match.group("end")
        + readme[match.end("end") :]
    )


def _find_marker_match(readme: str, marker: str) -> re.Match[str]:
    start = re.escape(f"<!-- README_BENCHMARK:{marker}:start -->")
    end = re.escape(f"<!-- README_BENCHMARK:{marker}:end -->")
    pattern = re.compile(
        rf"(?ms)(?P<start>^{start}[ \t]*$)(?P<body>.*?)(?P<end>^{end}[ \t]*$)"
    )
    match = pattern.search(readme)
    if not match:
        raise RuntimeError(f"README marker not found or malformed: {marker}")
    return match


def _replace_block_rows(
    readme: str,
    marker: str,
    replacement_map: dict[str, str],
) -> str:
    match = _find_marker_match(readme, marker)

    lines = _coerce_marker_body_lines(match.group("body"))
    matches = {key: [] for key in replacement_map}
    for idx, line in enumerate(lines):
        for key in replacement_map:
            if line.startswith(key):
                matches[key].append(idx)

    missing = [key for key, found in matches.items() if not found]
    duplicated = [key for key, found in matches.items() if len(found) > 1]
    if missing:
        raise RuntimeError(
            f"README marker {marker}: target row(s) not found: {', '.join(repr(k) for k in missing)}"
        )
    if duplicated:
        raise RuntimeError(
            f"README marker {marker}: ambiguous row match for {', '.join(repr(k) for k in duplicated)}"
        )

    out_lines = lines[:]
    for key, replacement in replacement_map.items():
        out_lines[matches[key][0]] = replacement

    updated = "\n".join(out_lines)

    return (
        readme[: match.start("start")]
        + match.group("start")
        + "\n"
        + updated
        + "\n"
        + match.group("end")
        + readme[match.end("end") :]
    )


def _parse_compile_time_report() -> dict[str, dict[str, dict[str, str]]]:
    if not COMPILE_TIME_REPORT.is_file():
        raise FileNotFoundError(f"missing {COMPILE_TIME_REPORT}")
    rows: dict[str, dict[str, dict[str, str]]] = {}
    for line in COMPILE_TIME_REPORT.read_text().splitlines():
        if not line.startswith("| "):
            continue
        if "|" not in line or line.startswith("| | name |"):
            continue
        cols = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cols) != 9:
            continue
        if cols[2] not in {"strict", "drop-in"}:
            continue
        rows.setdefault(cols[1], {})[cols[2]] = {
            "system_rss": cols[7],
            "psychicstd_rss": cols[8],
            "speedup": cols[5],
        }
    if not rows:
        raise ValueError(f"no compile-time rows parsed from {COMPILE_TIME_REPORT}")
    return rows


def _parse_mib(value: str) -> float | None:
    match = re.fullmatch(r"([0-9]+(?:\.[0-9]+)?) MiB", value)
    return float(match.group(1)) if match else None


def _reduction(base: str, improved: str) -> str:
    base_value = _parse_mib(base)
    improved_value = _parse_mib(improved)
    if base_value is None or improved_value is None:
        return "n/a"
    if base_value == 0:
        return "n/a"
    return f"{(base_value - improved_value) / base_value * 100:.0f}%"


def _parse_startup_speedup() -> str:
    if not STARTUP_REPORT.is_file():
        raise FileNotFoundError(f"missing {STARTUP_REPORT}")
    m = re.search(
        r"^Speedup:\s+\*\*(?P<speedup>[^*]+)\*\*\s+\(95% CI:",
        STARTUP_REPORT.read_text(),
        re.MULTILINE,
    )
    if not m:
        raise ValueError(f"no startup speedup line in {STARTUP_REPORT}")
    return m.group("speedup")


def main() -> int:
    compile_time = _parse_compile_time_report()
    startup_speedup = _parse_startup_speedup()

    wordcounter = compile_time.get("wordcounter", {})
    strict_speedup = wordcounter.get("strict", {}).get("speedup", "n/a")
    wordcounter_row = (
        "| [wordcounter](examples/wordcounter.cpp)| "
        f"[{strict_speedup}](compile_time.md) | "
        "Example application using the STL. Counts word occurrences in text files. |"
    )

    iostream = compile_time.get("iostream", {})
    peak_rows: list[str] = []
    for name, label in (
        ("wordcounter", "[wordcounter](examples/wordcounter.cpp)"),
        ("iostream", "[`<iostream>` test](benchmarks/compile_time/bench_iostream.cpp)"),
    ):
        row = iostream if name == "iostream" else wordcounter
        row_metrics = row.get("drop-in")
        if row_metrics is None:
            peak_rows.append(f"| {label} | n/a | n/a | n/a |")
            continue
        system_peak = row_metrics["system_rss"]
        psychicstd_peak = row_metrics["psychicstd_rss"]
        peak_rows.append(
            f"| {label} | {system_peak} | {psychicstd_peak} | "
            f"{_reduction(system_peak, psychicstd_peak)} |"
        )
    startup_line = (
        "Static linking also avoids the `libstdc++.so.6` and `libm.so.6` "
        f"dependencies. A representative Linux program measured "
        f"[{startup_speedup} faster exec-to-exit](startup.md), including loading, "
        "initialization, and its small fixed workload."
    )

    text = README.read_text()
    text = _replace_realworld_rows(text, "realworld-speedups")
    text = _replace_block_rows(
        text,
        "realworld-speedups",
        {"| [wordcounter](": wordcounter_row},
    )
    text = _replace_block_rows(
        text,
        "benchmark-peak-rss",
        {
            "| [wordcounter]": peak_rows[0],
            "| [`<iostream>` test]": peak_rows[1],
        },
    )
    text = _replace_block(text, "startup-speedup", startup_line)
    README.write_text(text)
    print(f"=== updated benchmark snippets in {README} ===")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001  # pragma: no cover - script entrypoint
        print(f"ERROR: benchmark README sync failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
