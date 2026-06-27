#!/bin/bash

# pipx install yamlfix
set -e
git ls-files | grep -E '\.ya?ml$' | xargs yamlfix
