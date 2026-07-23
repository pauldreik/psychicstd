#!/usr/bin/env bash
# Usage: run.sh [compiler]
#
# Builds a tiny Conan project that depends on fmt. The project is configured
# with a normal Conan profile plus the project-supplied psychic.profile
# overlay, so both the app and fmt itself are built with psychicstd.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REAL_CXX="${1:-${CXX:-clang++}}"
BUILD_DIR="$SCRIPT_DIR/build"
BUILD_PROFILE="$BUILD_DIR/build.profile"
HOST_PROFILE="$BUILD_DIR/host.profile"

if ! command -v conan >/dev/null 2>&1; then
  echo "conan is not installed; skipping example run" >&2
  exit 0
fi

if ! command -v "$REAL_CXX" >/dev/null 2>&1; then
  echo "compiler not found: $REAL_CXX" >&2
  exit 2
fi

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

COMPILER_MACROS="$("$REAL_CXX" -dM -E -x c++ /dev/null)"
if [[ "$COMPILER_MACROS" == *"__clang__"* ]]; then
  CONAN_COMPILER=clang
elif [[ "$COMPILER_MACROS" == *"__GNUC__"* ]]; then
  CONAN_COMPILER=gcc
else
  echo "unsupported compiler for example: $REAL_CXX" >&2
  exit 2
fi

if [[ "$CONAN_COMPILER" == gcc ]]; then
  major="$("$REAL_CXX" -dumpfullversion -dumpversion 2>/dev/null | sed -n 's/^\([0-9][0-9]*\).*/\1/p' | head -n 1)"
  if [[ -z "$major" ]]; then
    major="$("$REAL_CXX" --version | sed -n 's/.* \([0-9][0-9]*\)\..*/\1/p' | head -n 1)"
  fi
  if [[ -z "$major" || "$major" -lt 13 ]]; then
    echo "gcc 13+ is required for this example; found: $REAL_CXX" >&2
    exit 2
  fi
fi

CONAN_COMPILER_VERSION="$(
  "$REAL_CXX" -dumpversion 2>/dev/null | sed -n 's/^\([0-9][0-9]*\).*/\1/p' | head -n 1
)"
if [[ -z "$CONAN_COMPILER_VERSION" ]]; then
  CONAN_COMPILER_VERSION="$(
    "$REAL_CXX" --version | sed -n 's/.* \([0-9][0-9]*\)\..*/\1/p' | head -n 1
  )"
fi
if [[ -z "$CONAN_COMPILER_VERSION" ]]; then
  echo "could not determine compiler version from: $REAL_CXX" >&2
  exit 2
fi

cat >"$BUILD_PROFILE" <<EOF
[settings]
os=Linux
arch=x86_64
build_type=Release
compiler=$CONAN_COMPILER
compiler.version=$CONAN_COMPILER_VERSION
compiler.libcxx=libstdc++11
compiler.cppstd=gnu23
EOF

cat >"$HOST_PROFILE" <<EOF
include($BUILD_PROFILE)
include($SCRIPT_DIR/psychic.profile)
EOF

export CXX="$REAL_CXX"

conan build "$SCRIPT_DIR" \
  --output-folder "$BUILD_DIR" \
  --profile:build "$BUILD_PROFILE" \
  --profile:host "$HOST_PROFILE" \
  --build=missing

# Dependencies built with different psychicstd versions must not share binaries.
CONAN_GRAPH_ARGS=(
  "$SCRIPT_DIR"
  --profile:build "$BUILD_PROFILE"
  --profile:host "$HOST_PROFILE"
  --no-remote
  --format=json
  -vquiet
)
conan graph info "${CONAN_GRAPH_ARGS[@]}" \
  --out-file="$BUILD_DIR/graph.json"
conan graph info "${CONAN_GRAPH_ARGS[@]}" \
  --conf:host user.psychicstd:version=package-id-test \
  --out-file="$BUILD_DIR/graph-other-version.json"

fmt_package_id() {
  python3 - "$1" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as graph_file:
    nodes = json.load(graph_file)["graph"]["nodes"].values()

package_ids = [
    node["package_id"]
    for node in nodes
    if node["ref"].split("#", 1)[0].startswith("fmt/")
]
if len(package_ids) != 1:
    raise SystemExit(f"expected one fmt node, found {len(package_ids)}")
print(package_ids[0])
PY
}

default_package_id="$(fmt_package_id "$BUILD_DIR/graph.json")"
other_package_id="$(fmt_package_id "$BUILD_DIR/graph-other-version.json")"
if [[ "$default_package_id" == "$other_package_id" ]]; then
  echo "user.psychicstd:version did not affect fmt's package ID" >&2
  exit 1
fi

"$BUILD_DIR/build/Release/my_app"
