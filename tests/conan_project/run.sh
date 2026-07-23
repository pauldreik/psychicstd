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
PROFILE="$BUILD_DIR/profile.profile"

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

cat >"$PROFILE" <<EOF
include($SCRIPT_DIR/psychic.profile)

[settings]
os=Linux
arch=x86_64
build_type=Release
compiler=$CONAN_COMPILER
compiler.version=$CONAN_COMPILER_VERSION
compiler.libcxx=libstdc++11
compiler.cppstd=gnu23
EOF

export CXX="$REAL_CXX"

conan create "$SCRIPT_DIR" --profile:all "$PROFILE" --build=missing

conan install "$SCRIPT_DIR" \
  --output-folder "$BUILD_DIR" \
  --profile:all "$PROFILE" \
  --build=missing

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
  -DCMAKE_TOOLCHAIN_FILE="$BUILD_DIR/build/Release/generators/conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"
"$BUILD_DIR/my_app"
