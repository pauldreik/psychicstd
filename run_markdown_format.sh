#!/bin/bash

# sudo apt install mdformat
set -e
git ls-files | grep -E '\.md$' | xargs mdformat
