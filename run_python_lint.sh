#!/bin/bash

# pipx install ruff
set -e

ruff format
ruff check --fix
