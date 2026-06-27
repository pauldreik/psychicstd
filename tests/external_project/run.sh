#!/usr/bin/env bash
# Usage: run.sh [path/to/psychicstd]
# Defaults to two directories above this script if not given.
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PSYCHICSTD_ROOT="${1:-$(cd "$SCRIPT_DIR/../.." && pwd)}"
BUILD_DIR="$SCRIPT_DIR/build"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_FLAGS="-nostdinc++ -I$PSYCHICSTD_ROOT/include"
cmake --build "$BUILD_DIR"
"$BUILD_DIR/my_app"
