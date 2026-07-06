#!/bin/sh

set -eux

cd "$(dirname "$0")"

version=9.0.0
tarball=simdutf-$version.tar.gz
dirname=simdutf-$version
if [ ! -e $tarball ]; then
  wget -O $tarball "https://github.com/simdutf/simdutf/archive/refs/tags/v$version.tar.gz"
fi
echo "fd2ce975f29809a975a8da8843cfb3a7265af3f71be548f199d23cf65e101764  $tarball" | sha256sum -c -

PSYCHICHSTD="$(
  cd ../include
  pwd -P
)"

(
  tar xf $tarball
  cd $dirname
  cmake -B build -S . \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DSIMDUTF_FAST_TESTS=On \
    -DSIMDUTF_TOOLS=Off \
    -DSIMDUTF_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-std=c++20 -nostdinc++ -isystem $PSYCHICHSTD -D_PSYCHICSTD_COMPATIBILITY_LEVEL=0" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DBUILD_SHARED_LIBS=OFF
  cmake --build build -j"$(nproc)"
  ctest --test-dir build --output-on-failure -j"$(nproc)"
)
rm -rf $dirname

echo all good!
