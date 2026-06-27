#!/bin/bash

# sudo apt install mdformat
set -e
git ls-files | grep -E '(CMakeLists.txt|\.cmake)$' | xargs cmake-format -i
