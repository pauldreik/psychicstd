#!/bin/bash

# sudo apt install shfmt
set -e
git ls-files | grep -E '\.sh$' | xargs -n1 shfmt -w -i 2 -bn -ci
