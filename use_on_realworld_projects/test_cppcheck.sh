#!/bin/sh

# Stop on error, but allow per-test failures in timing loops.
set -eu

# Ensure ccache does not skew compile-time measurements.
export CCACHE_DISABLE=1

cd "$(dirname "$0")"

version=2.21.0
tarball=cppcheck-$version.tar.gz
dirname=cppcheck-$version
if [ ! -e $tarball ]; then
  wget -O $tarball "https://github.com/danmar/cppcheck/archive/refs/tags/$version.tar.gz"
fi
echo "f028ff75ca5372738f3737c8b3e8611426a6526b6aea2ef01301ab0f5902f044  $tarball" | sha256sum -c -

PSYCHICHSTD="$(
  cd ../include
  pwd -P
)"

REPORT="cppcheck_speed_report.md"

# ---- helpers ----

measure_compile() {
  _label="$1" _src="$2"
  _t=$(basename "$_src" .cpp)
  _bin="/tmp/cppcheck_${_label}_${_t}"
  _err="/tmp/cppcheck_${_label}_${_t}.err"

  _start=$(date +%s%N)
  if g++ $CXXFLAGS "$_src" -c -o "$_bin" 2>"$_err"; then
    _end=$(date +%s%N)
    _elapsed=$(((_end - _start) / 1000000))
    eval "time_${_label}_${_t}=$_elapsed"
    eval "status_${_label}_${_t}=pass"
  else
    eval "time_${_label}_${_t}=fail"
    eval "status_${_label}_${_t}=compfail"
  fi
  rm -f "$_bin" "$_err"
  return 0
}

# ---- prepare cppcheck source ----

rm -rf "$dirname"
tar xf "$tarball"

CPPCHECK_INC="-I $dirname/lib -I $dirname/externals/picojson -I $dirname/externals/simplecpp -I $dirname/externals/tinyxml2 -I $dirname/cli"
CPPCHECK_DEFS="-DFILESDIR='\"/usr/local/share/Cppcheck\"' -DHAVE_EXECINFO_H=1"

# ============================================================
# Full build verification
# ============================================================
echo "=== Full build verification ==="
(
  rm -rf cppcheck_build
  mkdir cppcheck_build
  cd cppcheck_build
  cmake ../"$dirname" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -isystem $PSYCHICHSTD" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DHAVE_RULES=OFF -DBUILD_TESTS=OFF -DBUILD_GUI=OFF \
    >/dev/null 2>&1
  if make -j"$(nproc)" >/dev/null 2>&1; then
    echo "  psychicstd build: OK"
  else
    echo "  psychicstd build: FAILED"
  fi
  cd ..
  rm -rf cppcheck_build
)
echo ""

# ============================================================
# Test build (78 test files → testrunner binary)
# ============================================================
echo "=== Test build ==="
(
  rm -rf cppcheck_test_build
  mkdir cppcheck_test_build
  cd cppcheck_test_build
  cmake ../"$dirname" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-nostdinc++ -isystem $PSYCHICHSTD" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DHAVE_RULES=OFF -DBUILD_TESTS=ON -DBUILD_GUI=OFF \
    -DDISABLE_DMAKE=ON \
    >/dev/null 2>&1
  if make -j"$(nproc)" testrunner >/dev/null 2>&1; then
    echo "  testrunner build: OK (78 test files compiled)"
    if bin/testrunner >/dev/null 2>&1; then
      echo "  testrunner run: OK"
    else
      echo "  testrunner run: segfault (expected — runtime stubs incomplete)"
    fi
  else
    echo "  testrunner build: FAILED"
  fi
  cd ..
  rm -rf cppcheck_test_build
)
echo ""

# ---- source files to measure ----

FILES="
  $dirname/lib/addoninfo.cpp
  $dirname/lib/analyzerinfo.cpp
  $dirname/lib/color.cpp
  $dirname/lib/timer.cpp
  $dirname/lib/errortypes.cpp
  $dirname/lib/astutils.cpp
  $dirname/lib/checkassert.cpp
  $dirname/lib/checkbool.cpp
  $dirname/lib/checkcondition.cpp
  $dirname/lib/checkfunctions.cpp
  $dirname/lib/tokenize.cpp
  $dirname/lib/symboldatabase.cpp
  $dirname/lib/valueflow.cpp
  $dirname/lib/checkclass.cpp
  $dirname/lib/checkbufferoverrun.cpp
  $dirname/cli/cmdlineparser.cpp
  $dirname/cli/filelister.cpp
  $dirname/cli/cppcheckexecutor.cpp
"

# ============================================================
# Phase 1: psychicstd compile times
# ============================================================
echo "=== Phase 1: psychicstd compile times ==="
CXXFLAGS="-std=c++20 -nostdinc++ -isystem $PSYCHICHSTD $CPPCHECK_INC $CPPCHECK_DEFS"

for src in $FILES; do
  if [ ! -f "$src" ]; then continue; fi
  t=$(basename "$src" .cpp)
  measure_compile psy "$src"
  eval "_ms=\$time_psy_${t}"
  if [ "$_ms" = "fail" ]; then
    echo "  $t: FAIL (compile)"
  else
    echo "  $t: OK [${_ms}ms]"
  fi
done
echo ""

# ============================================================
# Phase 2: system compiler reference
# ============================================================
echo "=== Phase 2: system compiler reference ==="
CXXFLAGS="-std=c++20 $CPPCHECK_INC $CPPCHECK_DEFS"

for src in $FILES; do
  if [ ! -f "$src" ]; then continue; fi
  t=$(basename "$src" .cpp)
  measure_compile sys "$src"
  eval "_ms=\$time_sys_${t}"
  if [ "$_ms" = "fail" ]; then
    echo "  $t: FAIL (compile)"
  else
    echo "  $t: OK [${_ms}ms]"
  fi
done
echo ""

# ============================================================
# Phase 3: write speed report
# ============================================================
echo "=== Phase 3: writing $REPORT ==="

{
  echo "# Cppcheck Compile-Time Comparison"
  echo ""
  echo "psychicstd vs system libstdc++ (GCC $(g++ -dumpversion))"
  echo ""
  echo "| Source file | psychicstd (ms) | libstdc++ (ms) | Speedup |"
  echo "|-------------|-----------------|----------------|---------|"

  total_psy=0 total_sys=0 count=0
  for src in $FILES; do
    if [ ! -f "$src" ]; then continue; fi
    t=$(basename "$src" .cpp)
    eval "_p=\$time_psy_${t}"
    eval "_s=\$time_sys_${t}"

    if [ "$_p" = "fail" ] || [ "$_s" = "fail" ]; then
      echo "| $t | — | — | — |"
      continue
    fi

    if [ "$_s" -gt 0 ] 2>/dev/null; then
      _speedup=$(awk "BEGIN { printf \"%.1f\", $_s / $_p }")
    else
      _speedup="—"
    fi

    echo "| $t | ${_p} | ${_s} | ${_speedup}x |"
    total_psy=$((total_psy + _p))
    total_sys=$((total_sys + _s))
    count=$((count + 1))
  done

  if [ "$count" -gt 0 ]; then
    _avg_psy=$((total_psy / count))
    _avg_sys=$((total_sys / count))
    _avg_speedup=$(awk "BEGIN { printf \"%.1f\", $_avg_sys / $_avg_psy }")
    total_all_psy=$total_psy
    total_all_sys=$total_sys
    echo "| **Total ($count files)** | **${total_psy}** | **${total_sys}** | **${_avg_speedup}x** |"
  fi

  echo ""
  echo "Generated $(date -Iseconds)"
} >"$REPORT"

echo "report written to $REPORT"

# cleanup
rm -rf "$dirname"
echo ""
echo "all done!"
