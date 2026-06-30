#!/bin/sh

# Stop on error, but allow per-test failures in Phase 2/3 loops.
set -eu

# Ensure ccache does not skew compile-time measurements.
export CCACHE_DISABLE=1

cd "$(dirname "$0")"

version=3.4.0
tarball=eigen-$version.tar.gz
dirname=eigen-$version
if [ ! -e $tarball ]; then
  wget -O $tarball "https://gitlab.com/libeigen/eigen/-/archive/$version/$tarball"
fi
echo "8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72  $tarball" | sha256sum -c -

PSYCHICHSTD="$(
  cd ../include
  pwd -P
)"

REPORT="eigen_speed_report.md"

# ---- helpers ----

# compile_and_run <label> <cxxflags> <ldflags> <testname>
# Sets global: time_<label>_<testname> = milliseconds (or "fail")
measure_compile() {
  _label="$1" _cxxflags="$2" _ldflags="$3" _t="$4"
  _src="$EIGEN_TEST_DIR/${_t}.cpp"
  _bin="/tmp/eigen_${_label}_${_t}"
  _err="/tmp/eigen_${_label}_${_t}.err"

  if [ ! -f "$_src" ]; then
    eval "time_${_label}_${_t}=skip"
    return 0
  fi

  _start=$(date +%s%N)
  if g++ $_cxxflags "$_src" $_ldflags -o "$_bin" 2>"$_err"; then
    _end=$(date +%s%N)
    _elapsed=$(((_end - _start) / 1000000))
    eval "time_${_label}_${_t}=$_elapsed"
    if "$_bin" >/dev/null 2>&1; then
      eval "status_${_label}_${_t}=pass"
    else
      eval "status_${_label}_${_t}=runfail"
    fi
  else
    eval "time_${_label}_${_t}=fail"
    eval "status_${_label}_${_t}=compfail"
  fi
  rm -f "$_bin" "$_err"
  return 0
}

# ---- prepare eigen source ----

rm -rf "$dirname"
tar xf "$tarball"

# Patch main.h — neutralise macros that clash with psychicstd
sed -i 's/^#define FORBIDDEN_IDENTIFIER/\/\/ DISABLED: &/' "$dirname/test/main.h"
sed -i 's/^#define B0 FORBIDDEN_IDENTIFIER/\/\/ DISABLED: &/' "$dirname/test/main.h"
sed -i 's/^#define I  FORBIDDEN_IDENTIFIER/\/\/ DISABLED: &/' "$dirname/test/main.h"

EIGEN_INCLUDE="$(pwd)/$dirname"
EIGEN_TEST_DIR="$EIGEN_INCLUDE/test"

# ============================================================
# Phase 1: comprehensive inline test (psychicstd only)
# ============================================================
echo "=== Phase 1: comprehensive inline test ==="
g++ -std=c++20 -nostdinc++ -isystem "$PSYCHICHSTD" \
  -I "$EIGEN_INCLUDE" \
  -x c++ - -x none \
  -nodefaultlibs -lsupc++ -lm -lc -lgcc_s -lgcc \
  -o /tmp/eigen_test <<'EOF'
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <Eigen/QR>
#include <Eigen/Cholesky>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <cassert>
#include <cmath>
#include <iostream>

void test_core() {
    Eigen::Matrix3d m;
    m << 1, 2, 3, 4, 5, 6, 7, 8, 10;
    assert(m(0,0) == 1 && m(2,2) == 10);
    assert(m.rows() == 3 && m.cols() == 3);
    assert(m.transpose()(0,1) == m(1,0));
    assert(m.trace() == 1+5+10);
    Eigen::Vector3d v(1,2,3), w(4,5,6);
    assert(v.dot(w) == 1*4+2*5+3*6);
    assert(m.sum() == 1+2+3+4+5+6+7+8+10);
    std::cout << "  Core: PASSED" << std::endl;
}

void test_geometry() {
    Eigen::AngleAxisd rot(M_PI/4, Eigen::Vector3d::UnitZ());
    Eigen::Quaterniond q(rot);
    assert(std::abs(q.norm() - 1.0) < 1e-12);
    Eigen::Vector3d vr = q * Eigen::Vector3d(1,0,0);
    double ex = std::cos(M_PI/4), ey = std::sin(M_PI/4);
    assert(std::abs(vr(0)-ex) < 1e-10 && std::abs(vr(1)-ey) < 1e-10);
    std::cout << "  Geometry: PASSED" << std::endl;
}

void test_lu() {
    Eigen::Matrix3d m;
    m << 1,2,3,4,5,6,7,8,10;
    Eigen::PartialPivLU<Eigen::Matrix3d> lu(m);
    Eigen::Vector3d x = lu.solve(Eigen::Vector3d(1,2,3));
    assert(std::abs((m*x)(0)-1) < 1e-10);
    assert(std::abs(lu.determinant()+3.0) < 1e-10);
    std::cout << "  LU: PASSED" << std::endl;
}

void test_qr() {
    Eigen::Matrix<double,4,3> m;
    m << 1,2,3,4,5,6,7,8,10,1,1,1;
    Eigen::HouseholderQR<Eigen::Matrix<double,4,3>> qr(m);
    Eigen::Matrix<double,4,4> Q = qr.householderQ() * Eigen::Matrix4d::Identity();
    Eigen::Matrix<double,4,3> R = qr.matrixQR().template triangularView<Eigen::Upper>();
    Eigen::Matrix<double,4,3> rec = Q * R;
    for (int i=0;i<4;i++) for (int j=0;j<3;j++) assert(std::abs(rec(i,j)-m(i,j)) < 1e-10);
    std::cout << "  QR: PASSED" << std::endl;
}

void test_cholesky() {
    Eigen::Matrix3d A;
    A << 4,2,2,2,5,3,2,3,6;
    Eigen::LLT<Eigen::Matrix3d> llt(A);
    assert(llt.info() == Eigen::Success);
    Eigen::Matrix3d L = llt.matrixL();
    Eigen::Matrix3d chk = L * L.transpose();
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) assert(std::abs(chk(i,j)-A(i,j)) < 1e-10);
    std::cout << "  Cholesky: PASSED" << std::endl;
}

void test_svd() {
    Eigen::Matrix<double,4,3> m;
    m << 1,2,3,4,5,6,7,8,10,1,1,1;
    Eigen::JacobiSVD<Eigen::Matrix<double,4,3>> svd(m, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Vector3d sigmas = svd.singularValues();
    assert(sigmas(0) > 0 && sigmas(0) >= sigmas(1) && sigmas(1) >= sigmas(2));
    Eigen::Matrix<double,4,3> rec = svd.matrixU().leftCols(3) * sigmas.asDiagonal() * svd.matrixV().transpose();
    for (int i=0;i<4;i++) for (int j=0;j<3;j++) assert(std::abs(rec(i,j)-m(i,j)) < 1e-8);
    std::cout << "  SVD: PASSED" << std::endl;
}

void test_eigenvalues() {
    Eigen::Matrix3d A;
    A << 4,2,2,2,5,3,2,3,6;
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(A);
    assert(es.info() == Eigen::Success);
    for (int i=0;i<3;i++) {
        Eigen::Vector3d v = es.eigenvectors().col(i);
        assert(std::abs((A*v-es.eigenvalues()(i)*v)(0)) < 1e-8);
    }
    std::cout << "  Eigenvalues: PASSED" << std::endl;
}

int main() {
    std::cout << "Phase 1: comprehensive inline test" << std::endl;
    test_core(); test_geometry(); test_lu(); test_qr();
    test_cholesky(); test_svd(); test_eigenvalues();
    std::cout << "Phase 1: all passed!" << std::endl;
    return 0;
}
EOF
/tmp/eigen_test
rm -f /tmp/eigen_test
echo "phase 1 passed"
echo ""

TEST_LIST="basicstuff meta numext block corners determinant diagonal array_cwise array_for_matrix constructor adjoint triangular"

# ============================================================
# Phase 2: compile Eigen tests with psychicstd, measure time
# ============================================================
echo "=== Phase 2: psychicstd compile times ==="
PSY_CXXFLAGS="-std=c++20 -nostdinc++ -isystem $PSYCHICHSTD -I $EIGEN_INCLUDE -I $EIGEN_TEST_DIR -DEIGEN_TEST_MAX_SIZE=320"
PSY_LDFLAGS="-nodefaultlibs -lsupc++ -lm -lc -lgcc_s -lgcc"

for t in $TEST_LIST; do
  measure_compile psy "$PSY_CXXFLAGS" "$PSY_LDFLAGS" "$t"
  eval "_t=\$time_psy_${t}"
  eval "_s=\$status_psy_${t}"
  if [ "$_t" = "fail" ]; then
    echo "  $t: FAIL (compile)"
  elif [ "$_s" = "runfail" ]; then
    echo "  $t: FAIL (runtime) [${_t}ms]"
  else
    echo "  $t: PASS [${_t}ms]"
  fi
done
echo ""

# ============================================================
# Phase 3: reference run with system compiler
# ============================================================
echo "=== Phase 3: system compiler reference ==="
SYS_CXXFLAGS="-std=c++20 -I $EIGEN_INCLUDE -I $EIGEN_TEST_DIR -DEIGEN_TEST_MAX_SIZE=320"
SYS_LDFLAGS=""

for t in $TEST_LIST; do
  measure_compile sys "$SYS_CXXFLAGS" "$SYS_LDFLAGS" "$t"
  eval "_t=\$time_sys_${t}"
  eval "_s=\$status_sys_${t}"
  if [ "$_t" = "fail" ]; then
    echo "  $t: FAIL (compile)"
  elif [ "$_s" = "runfail" ]; then
    echo "  $t: FAIL (runtime) [${_t}ms]"
  else
    echo "  $t: PASS [${_t}ms]"
  fi
done
echo ""

# ============================================================
# Phase 4: write speed report
# ============================================================
echo "=== Phase 4: writing $REPORT ==="

{
  echo "# Eigen Test Compile-Time Comparison"
  echo ""
  echo "psychicstd vs system libstdc++ (GCC $(g++ -dumpversion))"
  echo ""
  echo "| Test | psychicstd (ms) | libstdc++ (ms) | Speedup |"
  echo "|------|-----------------|----------------|---------|"

  total_psy=0 total_sys=0 count=0
  for t in $TEST_LIST; do
    eval "_p=\$time_psy_${t}"
    eval "_s=\$time_sys_${t}"
    eval "_ps=\$status_psy_${t}"
    eval "_ss=\$status_sys_${t}"

    if [ "$_p" = "fail" ] || [ "$_s" = "fail" ] || [ "$_p" = "skip" ] || [ "$_s" = "skip" ]; then
      echo "| $t | — | — | — |"
      continue
    fi

    if [ "$_s" -gt 0 ] 2>/dev/null; then
      _speedup=$(awk "BEGIN { printf \"%.1f\", $_s / $_p }")
    else
      _speedup="—"
    fi

    _pnote=""
    _snote=""
    [ "$_ps" = "runfail" ] && _pnote=" ⚠"
    [ "$_ss" = "runfail" ] && _snote=" ⚠"

    echo "| $t | ${_p}${_pnote} | ${_s}${_snote} | ${_speedup}x |"

    if [ "$_ps" = "pass" ] && [ "$_ss" = "pass" ]; then
      total_psy=$((total_psy + _p))
      total_sys=$((total_sys + _s))
      count=$((count + 1))
    fi
  done

  if [ "$count" -gt 0 ]; then
    _avg_psy=$((total_psy / count))
    _avg_sys=$((total_sys / count))
    _avg_speedup=$(awk "BEGIN { printf \"%.1f\", $_avg_sys / $_avg_psy }")
    echo "| **Average ($count tests)** | **${_avg_psy}** | **${_avg_sys}** | **${_avg_speedup}x** |"
  fi

  echo ""
  echo "⚠ = compiled but test run failed (not included in average)"
  echo ""
  echo "Generated $(date -Iseconds)"
} >"$REPORT"

echo "report written to $REPORT"

# cleanup
rm -rf "$dirname"

echo ""
echo "all done!"
