#!/bin/bash
# Measure and compare compile time of example.cpp with psychicstd vs the
# system standard library (libstdc++), and profile where the frontend spends
# its time using clang's -ftime-trace.
#
# Usage: ./measure.sh
#
# Requires: clang++-21, python3.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../.." && pwd)"
INCLUDE="$REPO_ROOT/include"

CXX=${CXX:-clang++-21}
STD=${STD:-c++20}
N=${N:-10}
SRC="$HERE/example.cpp"

PSYCHIC_FLAGS=(-std=$STD -nostdinc++ -isystem "$INCLUDE")
SYSTEM_FLAGS=(-std=$STD)

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

echo "compiler: $($CXX --version | head -1)"
echo "standard: $STD, runs per config: $N"
echo

# --- 1. sanity: both must compile ---------------------------------------------
$CXX "${PSYCHIC_FLAGS[@]}" -c "$SRC" -o /dev/null
$CXX "${SYSTEM_FLAGS[@]}" -c "$SRC" -o /dev/null

# --- 2. timing ----------------------------------------------------------------
time_config() {
  local out=$1
  shift
  : >"$out"
  for _ in $(seq "$N"); do
    /usr/bin/time -f "%e" "$CXX" "$@" -c "$SRC" -o /dev/null 2>>"$out"
  done
}
time_config "$WORK/psychic.times" "${PSYCHIC_FLAGS[@]}"
time_config "$WORK/system.times" "${SYSTEM_FLAGS[@]}"

echo "=== compile time (seconds, median of $N) ==="
python3 - "$WORK/psychic.times" "$WORK/system.times" <<'PY'
import statistics as s, sys
p = s.median(float(x) for x in open(sys.argv[1]))
y = s.median(float(x) for x in open(sys.argv[2]))
print(f"  psychicstd      : {p:.3f}")
print(f"  system libstdc++: {y:.3f}")
print(f"  speedup         : {y / p:.2f}x")
PY
echo

# --- 3. preprocessed size (how much code the parser sees) ----------------------
echo "=== preprocessed code size ==="
size_config() {
  local label=$1
  shift
  local lines bytes
  lines=$("$CXX" "$@" -E "$SRC" | grep -v '^#' | grep -cv '^[[:space:]]*$')
  bytes=$("$CXX" "$@" -E "$SRC" | wc -c)
  printf "  %-16s: %6s non-blank lines, %8s bytes\n" "$label" "$lines" "$bytes"
}
size_config "psychicstd" "${PSYCHIC_FLAGS[@]}"
size_config "system libstdc++" "${SYSTEM_FLAGS[@]}"
echo

# --- 4. -ftime-trace frontend breakdown ---------------------------------------
$CXX "${PSYCHIC_FLAGS[@]}" -ftime-trace -c "$SRC" -o "$WORK/psychic.o"
$CXX "${SYSTEM_FLAGS[@]}" -ftime-trace -c "$SRC" -o "$WORK/system.o"

echo "=== -ftime-trace: top frontend categories (ms) ==="
python3 - "$WORK/psychic.json" "$WORK/system.json" <<'PY'
import json, sys


def totals(fn):
    ev = json.load(open(fn))["traceEvents"]
    t = {}
    for e in ev:
        n = e.get("name", "")
        if n.startswith("Total "):
            t[n[6:]] = e.get("dur", 0) / 1000.0
    files = len({e["args"]["detail"] for e in ev
                 if e.get("name") == "Source" and "detail" in e.get("args", {})})
    return t, files


keys = ["ExecuteCompiler", "Frontend", "Source", "ParseClass",
        "InstantiateFunction", "InstantiateClass",
        "PerformPendingInstantiations", "Backend"]
pt, pf = totals(sys.argv[1])
st, sf = totals(sys.argv[2])
print(f"  {'category':<30}{'psychicstd':>12}{'system':>10}")
for k in keys:
    print(f"  {k:<30}{pt.get(k, 0):>11.1f}{st.get(k, 0):>10.1f}")
print(f"  {'distinct files parsed':<30}{pf:>11}{sf:>10}")
PY
