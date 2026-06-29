#!/bin/sh

set -eux

cd "$(dirname "$0")"

version=3.12.0
tarball=nlohmann-$version.tar.gz
dirname=json-$version
if [ ! -e $tarball ]; then
  wget -O $tarball "https://github.com/nlohmann/json/archive/refs/tags/v$version.tar.gz"
fi
echo "4b92eb0c06d10683f7447ce9406cb97cd4b453be18d7279320f7b2f025c10187  $tarball" | sha256sum -c -
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
  cd $dirname
  cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-std=c++20 -nostdinc++ -isystem $PSYCHICHSTD" \
    -DCMAKE_EXE_LINKER_FLAGS="-nodefaultlibs" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lsupc++ -lm -lc -lgcc_s -lgcc" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DBUILD_SHARED_LIBS=OFF \
    -DJSON_TestStandards=20 \
    -DJSON_BuildTests=OFF
  cmake --build build -j"$(nproc)"
  rm -rf build

  # Build all 217 API examples from the documentation
  NLOHMANN_INCLUDE="$(pwd)/include"
  failed=0
  for f in docs/mkdocs/docs/examples/*.cpp; do
    if ! g++ -std=c++20 -nostdinc++ -isystem "$PSYCHICHSTD" \
      -I "$NLOHMANN_INCLUDE" \
      "$f" -nodefaultlibs -lsupc++ -lm -lc -lgcc_s -lgcc \
      -o /tmp/nlohmann_example_test 2>/dev/null; then
      echo "FAIL: $f" >&2
      failed=$((failed + 1))
    fi
  done
  if [ "$failed" -ne 0 ]; then
    echo "$failed example(s) failed to compile" >&2
    exit 1
  fi
  echo "All examples compiled successfully"
)
rm -rf $dirname

echo all good!
