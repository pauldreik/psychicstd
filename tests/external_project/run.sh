#!/usr/bin/env bash
# Usage: run.sh [path/to/psychicstd]
# Defaults to two directories above this script if not given.
#
# Simulates an independent CMake project consuming psychicstd.  Flags are
# injected at configure time so the project's own CMakeLists.txt stays
# untouched.
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PSYCHICSTD_ROOT="${1:-$(cd "$SCRIPT_DIR/../.." && pwd)}"
BUILD_DIR="$SCRIPT_DIR/build"

# we can not remove the build dir when running the docker tests,
# delete everything inside it instead
rm -rf "$BUILD_DIR/*"
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_COMPILER_WORKS=1 \
  -DCMAKE_CXX_FLAGS="-nostdinc++ -fvisibility=hidden -I$PSYCHICSTD_ROOT/include" \
  -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
  -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc"
cmake --build "$BUILD_DIR"
"$BUILD_DIR/my_app"
