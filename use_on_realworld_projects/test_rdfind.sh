#!/bin/sh

set -eux

tarball=rdfind-1.8.0.tar.gz
if [ ! -e $tarball ] ; then
    wget 'https://github.com/pauldreik/rdfind/releases/download/releases%2F1.8.0/$tarball'
fi
echo "0a2d0d32002cc2dc0134ee7b649bcc811ecfb2f8d9f672aa476a851152e7af35  $tarball" |sha256sum -c -

CXX=c++

# where is libsup++?
LIBSUP=$(c++ -print-file-name=libsupc++.a)
if [ ! -e $LIBSUP ]; then
     echo could not find LIBSUP
fi
PSYCHICHSTD="$(cd ../include ; pwd -P)"
export CXXFLAGS="-nostdinc++ -nodefaultlibs -isystem $PSYCHICHSTD"
export LDFLAGS="-Wl,--start-group -lm -lc -lgcc_s -lgcc -Wl,--end-group"


tar xf $tarball
cd rdfind-1.8.0
./configure
make
make check
