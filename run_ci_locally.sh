#!/usr/bin/env bash
# Run all CI-equivalent configurations locally.
# Each job uses a dedicated build dir so incremental rebuilds are fast.
#
# Usage:
#   ./run_ci_locally.sh               # run all jobs
#   ./run_ci_locally.sh clang asan    # run a specific job by name (words joined, substring match)
#   ./run_ci_locally.sh clang         # run all jobs whose name contains "clang"

set -uo pipefail

REPO=$(cd "$(dirname "$0")" && pwd)
NPROC=$(nproc)

RED='\033[31m'
GREEN='\033[32m'
BOLD='\033[1m'
DIM='\033[2m'
RESET='\033[0m'

export ASAN_OPTIONS=halt_on_error=1:abort_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1:print_stacktrace=1

declare -a RESULTS
FAILED=0
FILTER="$*" # join all args so "clang asan" matches the job named "clang asan"

match() {
  [[ -z "$FILTER" ]] && return 0
  [[ "$1" == *"$FILTER"* ]]
}

DOCKER_DOCKERFILE="$REPO/.docker/ci-gcc.dockerfile"

# Build a gcc CI image if it isn't cached yet.
# Image name convention: psychicstd-ci:gcc-NN  (NN = GCC major version)
ensure_docker_image() {
  local image="$1"
  docker image inspect "$image" &>/dev/null && return 0
  local gcc_ver="${image##*:gcc-}"
  echo -e "  ${DIM}building $image (one-time, will be cached)...${RESET}"
  docker build \
    --build-arg "GCC_VERSION=$gcc_ver" \
    -t "$image" \
    -f "$DOCKER_DOCKERFILE" \
    "$REPO"
}

run_docker() {
  local name="$1" image="$2"
  shift 2
  match "$name" || return 0
  echo -e "\n${BOLD}─── $name ${DIM}[docker: $image]${RESET}"
  local status=pass
  if ! ensure_docker_image "$image"; then
    status="FAIL (image build)"
  else
    # Shell-quote each cmake arg so flags with spaces survive the bash -c boundary.
    local cmake_args
    cmake_args=$(printf ' %q' "$@")
    # The ext_project test builds inside the source tree (tests/external_project/build/).
    # Shadow that path with a fresh writable tmpdir so the host's CMakeCache.txt
    # (which has host paths) doesn't collide with the container's /src paths.
    local ext_build
    ext_build=$(mktemp -d)
    if ! docker run --rm \
      -v "$REPO":/src:ro \
      -v "$ext_build":/src/tests/external_project/build \
      "$image" \
      bash -c "
                    set -euo pipefail
                    # Clean up the bind-mounted dir on exit so the host (non-root) can rmdir it.
                    trap 'find /src/tests/external_project/build -mindepth 1 -depth -delete 2>/dev/null || true' EXIT
                    cmake -S /src -B /tmp/build$cmake_args
                    cmake --build /tmp/build -j\$(nproc)
                    ctest --test-dir /tmp/build --output-on-failure
                "; then
      status=FAIL
    fi
    rmdir "$ext_build" 2>/dev/null || rm -rf "$ext_build" 2>/dev/null || true
  fi
  if [[ $status == pass ]]; then
    RESULTS+=("${GREEN}pass${RESET}  $name")
  else
    RESULTS+=("${RED}$status${RESET}  $name")
    FAILED=1
  fi
}

run() {
  local name="$1" buildrel="$2" build="$REPO/$2"
  shift 2
  match "$name" || return 0
  echo -e "\n${BOLD}─── $name ${DIM}[$buildrel]${RESET}"
  local status=pass
  if ! cmake -S "$REPO" -B "$build" "$@"; then
    status="FAIL (configure)"
  elif ! cmake --build "$build" -j"$NPROC"; then
    status="FAIL (build)"
  elif ! ctest --test-dir "$build" --output-on-failure; then
    status="FAIL (test)"
  fi
  if [[ $status == pass ]]; then
    RESULTS+=("${GREEN}pass${RESET}  $name")
  else
    RESULTS+=("${RED}$status${RESET}  $name")
    FAILED=1
  fi
}

have() { command -v "$1" &>/dev/null; }

# Return the highest versioned clang++ found (e.g. clang++-21), falling back to
# plain clang++ if no versioned binary exists.
find_clangxx() {
  local v
  for v in $(seq 30 -1 14); do
    have "clang++-$v" && {
      echo "clang++-$v"
      return 0
    }
  done
  have clang++ && {
    echo "clang++"
    return 0
  }
  return 1
}

if have g++; then
  run "gcc debug" build_gcc_debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
  run "gcc release" build_gcc_release -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
else
  echo "skip gcc: g++ not found"
fi

if CLANGXX=$(find_clangxx); then
  run "clang debug" build_clang_debug -DCMAKE_CXX_COMPILER="$CLANGXX" -DCMAKE_BUILD_TYPE=Debug
  run "clang release" build_clang_release -DCMAKE_CXX_COMPILER="$CLANGXX" -DCMAKE_BUILD_TYPE=Release
  run "clang asan" build_clang_asan \
    -DCMAKE_CXX_COMPILER="$CLANGXX" \
    -DCMAKE_BUILD_TYPE=Debug \
    "-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer" \
    "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address"
  run "clang ubsan" build_clang_ubsan \
    -DCMAKE_CXX_COMPILER="$CLANGXX" \
    -DCMAKE_BUILD_TYPE=Debug \
    "-DCMAKE_CXX_FLAGS=-fsanitize=undefined" \
    "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=undefined"
else
  echo "skip clang: no clang++ found"
fi

if have docker; then
  run_docker "docker gcc-12 debug" "psychicstd-ci:gcc-14" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
  run_docker "docker gcc-12 release" "psychicstd-ci:gcc-14" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release

  run_docker "docker gcc-14 debug" "psychicstd-ci:gcc-14" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
  run_docker "docker gcc-14 release" "psychicstd-ci:gcc-14" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release

  run_docker "docker gcc-16 debug" "psychicstd-ci:gcc-16" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
  run_docker "docker gcc-16 release" "psychicstd-ci:gcc-16" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
else
  echo "skip docker: docker not found"
fi

if [[ ${#RESULTS[@]} -gt 0 ]]; then
  echo -e "\n${BOLD}Results:${RESET}"
  for r in "${RESULTS[@]}"; do
    echo -e "  $r"
  done
fi

exit "$FAILED"
