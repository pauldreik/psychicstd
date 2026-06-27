#!/bin/bash

# sudo apt install shfmt
set -e
git ls-files | grep -E '\.sh$' | xargs -n1 shfmt --write -i 2 -bn -ci
