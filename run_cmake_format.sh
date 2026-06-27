#!/bin/bash

# pipx install gersemi
set -e
#SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
#"${SCRIPT_DIR}/.venv/bin/gersemi" -i $(git ls-files | grep -E '(CMakeLists\.txt|\.cmake)$')

git ls-files \
  | grep -E '(CMakeLists\.txt|\.cmake)$' \
  | xargs gersemi -i
