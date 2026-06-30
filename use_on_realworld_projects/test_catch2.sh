#!/bin/sh

# Ensure ccache does not skew compile-time measurements.
export CCACHE_DISABLE=1

set -eu

cd "$(dirname "$0")"

tarball=Catch2-v3.8.0.tar.gz
if [ ! -e $tarball ]; then
  wget -O $tarball "https://github.com/catchorg/Catch2/archive/refs/tags/v3.8.0.tar.gz"
fi
echo "1ab2de20460d4641553addfdfe6acd4109d871d5531f8f519a52ea4926303087  $tarball" | sha256sum -c -

PSYCHICHSTD="$(
  cd ../include
  pwd -P
)"

REPORT="catch2_speed_report.md"

# ============================================================
# Phase 1: psychicstd build + test
# ============================================================
echo "=== Phase 1: psychicstd build ==="
rm -rf Catch2-3.8.0
tar xf "$tarball"

PSY_START=$(date +%s%N)
(
  cd Catch2-3.8.0
  cmake -B build --preset basic-tests \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-std=c++20 -nostdinc++ -isystem $PSYCHICHSTD" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DCATCH_ENABLE_WERROR=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    >/dev/null 2>&1
  cmake --build build -j"$(nproc)" >/dev/null 2>&1
)
PSY_END=$(date +%s%N)
PSY_BUILD_MS=$(((PSY_END - PSY_START) / 1000000))
echo "  build: ${PSY_BUILD_MS}ms"

echo "  running tests..."
(
  cd Catch2-3.8.0
  ctest --test-dir build --output-on-failure -E "ApprovalTests" >/tmp/catch2_psy_ctest.log 2>&1
)
PSY_PASSED=$(grep 'tests passed' /tmp/catch2_psy_ctest.log | tail -1 | grep -o '[0-9]\+' | tail -1 || echo 0)
PSY_FAILED=$(grep 'tests failed' /tmp/catch2_psy_ctest.log | tail -1 | grep -o '[0-9]\+' | tail -1 || echo 0)
echo "  ${PSY_PASSED} passed, ${PSY_FAILED} failed"
rm -rf Catch2-3.8.0/build

# ============================================================
# Phase 2: system compiler reference build
# ============================================================
echo ""
echo "=== Phase 2: system compiler reference ==="
rm -rf Catch2-3.8.0
tar xf "$tarball"

SYS_START=$(date +%s%N)
(
  cd Catch2-3.8.0
  cmake -B build --preset basic-tests \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-std=c++20" \
    -DCATCH_ENABLE_WERROR=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    >/dev/null 2>&1
  cmake --build build -j"$(nproc)" >/dev/null 2>&1
)
SYS_END=$(date +%s%N)
SYS_BUILD_MS=$(((SYS_END - SYS_START) / 1000000))
echo "  build: ${SYS_BUILD_MS}ms"

rm -rf Catch2-3.8.0

# ============================================================
# Phase 3: write speed report
# ============================================================
echo ""
echo "=== Phase 3: writing $REPORT ==="

if [ "$SYS_BUILD_MS" -gt 0 ] 2>/dev/null; then
  SPEEDUP=$(awk "BEGIN { printf \"%.1f\", $SYS_BUILD_MS / $PSY_BUILD_MS }")
else
  SPEEDUP="—"
fi

{
  echo "# Catch2 Compile-Time Comparison"
  echo ""
  echo "psychicstd vs system libstdc++ (GCC $(g++ -dumpversion))"
  echo ""
  echo "| Metric | psychicstd | libstdc++ | Speedup |"
  echo "|--------|-----------|-----------|---------|"
  echo "| Build time | ${PSY_BUILD_MS}ms | ${SYS_BUILD_MS}ms | ${SPEEDUP}x |"
  echo "| Tests passed | ${PSY_PASSED} | — | — |"
  echo "| Tests failed | ${PSY_FAILED} | — | — |"
  echo ""
  echo "ApprovalTests excluded (stdout/stderr ordering differs with psychicstd's buffered cerr/clog)."
  echo ""
  echo "Generated $(date -Iseconds)"
} >"$REPORT"

echo "report written to $REPORT"

rm -rf Catch2-3.8.0
echo ""
echo "all done!"
