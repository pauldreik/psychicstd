#!/bin/sh

set -eux

cd "$(dirname "$0")"

version=11.1.4
tarball=fmt-$version.tar.gz
dirname=fmt-$version
if [ ! -e $tarball ]; then
  wget -O $tarball "https://github.com/fmtlib/fmt/archive/refs/tags/$version.tar.gz"
fi
echo "ac366b7b4c2e9f0dde63a59b3feb5ee59b67974b14ee5dc9ea8ad78aa2c1ee1e  $tarball" | sha256sum -c -

PSYCHICHSTD="$(
  cd ../include
  pwd -P
)"

(
  tar xf $tarball
  cd $dirname
  cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-DFMT_USE_LOCALE=0 -std=c++20 -nostdinc++ -isystem $PSYCHICHSTD" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DFMT_DOC=OFF \
    -DFMT_TEST=OFF \
    -DFMT_INSTALL=OFF
  cmake --build build -j"$(nproc)"

  FMT_INCLUDE="$(pwd)/include"
  g++ -std=c++20 -nostdinc++ -isystem "$PSYCHICHSTD" \
    -DFMT_USE_LOCALE=0 \
    -I "$FMT_INCLUDE" \
    -x c++ - -x none "$FMT_INCLUDE"/../build/libfmtd.a \
    -nodefaultlibs -lsupc++ -lm -lc -lgcc_s -lgcc \
    -o /tmp/fmt_test <<'EOF'
#include <cassert>
#include <fmt/core.h>
#include <string>

int main() {
    auto s = fmt::format("hello {}!", "psychicstd");
    assert(s == "hello psychicstd!");

    auto n = fmt::format("{} + {} = {}", 2, 3, 5);
    assert(n == "2 + 3 = 5");

    return 0;
}
EOF
  /tmp/fmt_test
  rm -f /tmp/fmt_test
  rm -rf build
  echo "fmt test passed"
)
rm -rf $dirname

echo all good!
