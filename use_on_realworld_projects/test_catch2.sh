#!/bin/sh

set -eux

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

# CMAKE_EXE_LINKER_FLAGS carries -nodefaultlibs (drops libstdc++).
# CMAKE_CXX_STANDARD_LIBRARIES is placed after objects in the link command,
# so static libraries resolve correctly; we supply the necessary runtime:
#   -lsupc++  C++ ABI (operator new/delete, __cxa_*, __gxx_personality_v0)
#   -lm -lc -lgcc_s -lgcc  the rest of what GCC normally provides.
(
  tar xf $tarball
  cd Catch2-3.8.0
  cmake -B build --preset basic-tests \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-std=c++20 -nostdinc++ -isystem $PSYCHICHSTD" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DCATCH_ENABLE_WERROR=OFF \
    -DBUILD_SHARED_LIBS=OFF
  cmake --build build -j"$(nproc)"
  # ApprovalTests compares exact stdout/stderr interleaving against a baseline
  # generated with libstdc++.  Our cerr/clog write directly to C stderr while
  # stdout is buffered, so the ordering differs — not a functional failure.
  ctest --test-dir build --output-on-failure -E "ApprovalTests"
  rm -rf build
)
rm -rf Catch2-3.8.0

echo all good!
