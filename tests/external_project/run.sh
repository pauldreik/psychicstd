#!/usr/bin/env bash
# Usage: run.sh [path/to/psychicstd] [mode] [compiler]
# Defaults to two directories above this script if not given.
#
# Simulates an independent CMake project consuming psychicstd.  The project
# itself stays untouched; the toolchain overlay is selected at configure time.
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PSYCHICSTD_ROOT="${1:-$(cd "$SCRIPT_DIR/../.." && pwd)}"
MODE="${2:-toolchain}"
CXX_COMPILER="${3:-}"
BUILD_ROOT="$SCRIPT_DIR/build"
BUILD_DIR="$BUILD_ROOT/$MODE"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_ROOT"

cmake_args=(
  -S "$SCRIPT_DIR"
  -B "$BUILD_DIR"
  -DCMAKE_CXX_STANDARD=23
)

if [[ -n "$CXX_COMPILER" ]]; then
  cmake_args+=(-DCMAKE_CXX_COMPILER="$CXX_COMPILER")
fi

case "$MODE" in
  toolchain)
    cmake_args+=(-DCMAKE_TOOLCHAIN_FILE="$PSYCHICSTD_ROOT/cmake/psychicstd-toolchain.cmake")
    ;;
  toolchain-asan)
    cmake_args+=(
      -DCMAKE_TOOLCHAIN_FILE="$PSYCHICSTD_ROOT/cmake/psychicstd-toolchain.cmake"
      "-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer"
      "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address"
    )
    ;;
  toolchain-ubsan)
    cmake_args+=(
      -DCMAKE_TOOLCHAIN_FILE="$PSYCHICSTD_ROOT/cmake/psychicstd-toolchain.cmake"
      "-DCMAKE_CXX_FLAGS=-fsanitize=undefined"
      "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=undefined"
    )
    ;;
  toolchain-asan-ubsan)
    cmake_args+=(
      -DCMAKE_TOOLCHAIN_FILE="$PSYCHICSTD_ROOT/cmake/psychicstd-toolchain.cmake"
      "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer"
      "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined"
    )
    ;;
  *)
    echo "unknown mode: $MODE" >&2
    exit 2
    ;;
esac

cmake "${cmake_args[@]}"
# Reconfiguring an existing build must apply the overlay again.
cmake "${cmake_args[@]}"
cmake --build "$BUILD_DIR"

run_env=()
case "$MODE" in
  toolchain-asan)
    run_env+=(
      "ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:abort_on_error=1${ASAN_OPTIONS:+:$ASAN_OPTIONS}"
    )
    ;;
  toolchain-ubsan)
    run_env+=(
      "UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1:print_stacktrace=1${UBSAN_OPTIONS:+:$UBSAN_OPTIONS}"
    )
    ;;
  toolchain-asan-ubsan)
    run_env+=(
      "ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:abort_on_error=1${ASAN_OPTIONS:+:$ASAN_OPTIONS}"
      "UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1:print_stacktrace=1${UBSAN_OPTIONS:+:$UBSAN_OPTIONS}"
    )
    ;;
esac

env "${run_env[@]}" "$BUILD_DIR/my_app"
