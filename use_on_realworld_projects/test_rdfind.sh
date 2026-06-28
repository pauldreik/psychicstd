#!/bin/sh

set -eux

cd "$(dirname "$0")"

tarball=rdfind-1.8.0.tar.gz
if [ ! -e $tarball ]; then
  wget "https://github.com/pauldreik/rdfind/releases/download/releases%2F1.8.0/$tarball"
fi
echo "0a2d0d32002cc2dc0134ee7b649bcc811ecfb2f8d9f672aa476a851152e7af35  $tarball" | sha256sum -c -

export CXX=c++

## first - ensure we can build normally.
#(
#  tar xf $tarball
#  cd rdfind-1.8.0
#  ./configure
#  make
#  make check
#  make distclean
#)

PSYCHICHSTD="$(
  cd ../include
  pwd -P
)"
export CXXFLAGS="-std=c++20 -nostdinc++ -isystem $PSYCHICHSTD"
# LDFLAGS and LIBS are passed as configure arguments so they are baked into
# the Makefile - plain "make" then works without extra flags.
# Configure's own linker tests also use them; providing a complete runtime
# here means those tests still succeed with -nodefaultlibs.

(
  tar xf $tarball
  cd rdfind-1.8.0
  ./configure LDFLAGS="-nodefaultlibs" LIBS="-lsupc++ -lm -lc -lgcc_s -lgcc"
  make
  make check
  make distclean
)

echo all good!
