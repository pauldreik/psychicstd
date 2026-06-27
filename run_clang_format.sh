#!/bin/bash
set -e
git ls-files | grep -E '(^include/[^/]+$|\.(h|hpp|cpp|cc|c)$)' | xargs clang-format-21 -i
