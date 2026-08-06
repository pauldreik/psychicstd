#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "usage: $0 QUICK|FULL|INACCURATE_BUT_FAST" >&2
  exit 2
fi

case $1 in
  QUICK)
    realworld_args=(--build-type debug --time-budget 1h)
    benchmark_args=(--reps 10)
    startup_samples=(BENCH_N=300 BENCH_REPS=3)
    cmake_parallel=()
    ;;
  FULL)
    realworld_args=(--build-type both --max-reps 9 --time-budget 7h)
    benchmark_args=(--reps 10)
    startup_samples=(BENCH_N=300 BENCH_REPS=3)
    cmake_parallel=()
    ;;
  INACCURATE_BUT_FAST)
    parallel_jobs=$(nproc)
    realworld_args=(
      --build-type debug
      --reps 1
      --jobs "${parallel_jobs}"
      --enable-ccache
    )
    benchmark_args=(--reps 1 --enable-ccache)
    startup_samples=(BENCH_N=1 BENCH_REPS=1 BENCH_INACCURATE=1)
    cmake_parallel=(--parallel "${parallel_jobs}")
    ;;
  *)
    echo "usage: $0 QUICK|FULL|INACCURATE_BUT_FAST" >&2
    exit 2
    ;;
esac

if [[ $1 == INACCURATE_BUT_FAST ]]; then
  unset CCACHE_DISABLE
  export PATH="/usr/lib/ccache:${PATH}"
else
  export CCACHE_DISABLE=1
fi

run_and_format() {
  local command=("$@")
  local status=0
  "${command[@]}"
  status=$?
  if [[ ${status} -ne 0 ]]; then
    echo "ERROR: command failed: ${command[*]}" >&2
    return "${status}"
  fi
  ./run_markdown_format.sh
  status=$?
  if [[ ${status} -ne 0 ]]; then
    echo "ERROR: run_markdown_format.sh failed after: ${command[*]}" >&2
    return "${status}"
  fi
  return 0
}

echo "==> Real-world project benchmarks"
run_and_format scripts/generate_realworld_benchmark_reports.py "${realworld_args[@]}"

echo "==> Public-header compile costs"
run_and_format scripts/benchmark_header_cost.py "${benchmark_args[@]}"

echo "==> Focused compile-time and peak-RSS benchmarks"
run_and_format scripts/benchmark_compile_time.py "${benchmark_args[@]}"

echo "==> Process startup benchmark"
cmake -B build/ -S . -DCMAKE_BUILD_TYPE=Debug
run_and_format env "${startup_samples[@]}" cmake --build build/ \
  --target startup_bench "${cmake_parallel[@]}"

echo "==> Sync README benchmark snippets"
run_and_format scripts/update_readme_from_benchmarks.py

echo "All benchmark reports updated."
