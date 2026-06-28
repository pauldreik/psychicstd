#!/bin/bash
set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd -P)"
cd "$SCRIPT_DIR"

tarball=rdfind-1.8.0.tar.gz
if [ ! -e "$tarball" ]; then
  wget "https://github.com/pauldreik/rdfind/releases/download/releases%2F1.8.0/$tarball"
fi
echo "0a2d0d32002cc2dc0134ee7b649bcc811ecfb2f8d9f672aa476a851152e7af35  $tarball" | sha256sum -c -

PSYCHICSTD="$(cd "$SCRIPT_DIR/../include" && pwd -P)"
export CXX=c++
export CCACHE_DISABLE=1

# Run one build variant. Timings written to $SCRIPT_DIR/<name>_r<run>_{configure,make,check}.ms
# Usage: run_build <name> <run> <cxxflags> [extra configure args...]
run_build() {
  local name="$1" run="$2" cxxflags="$3"
  shift 3
  local t0
  echo "=== Building $name (run $run) ==="
  rm -rf rdfind-1.8.0
  tar xf "$tarball"
  cd rdfind-1.8.0
  export CXXFLAGS="$cxxflags"

  t0=$(date +%s%3N)
  ./configure "$@"
  echo $(($(date +%s%3N) - t0)) >"../${name}_r${run}_configure.ms"

  t0=$(date +%s%3N)
  make
  echo $(($(date +%s%3N) - t0)) >"../${name}_r${run}_make.ms"

  t0=$(date +%s%3N)
  make check
  echo $(($(date +%s%3N) - t0)) >"../${name}_r${run}_check.ms"

  make distclean
  cd "$SCRIPT_DIR"
}

# Interleave variants across rounds so all get similar cache conditions per round.
for run in 1 2 3; do
  echo "=== Round $run/3 ==="
  run_build sys_o0 "$run" "-std=c++20 -O0"
  run_build sys_o2 "$run" "-std=c++20 -O2"
  run_build psy_o0 "$run" "-std=c++20 -O0 -nostdinc++ -isystem $PSYCHICSTD" \
    LDFLAGS="-nodefaultlibs" LIBS="-lsupc++ -lm -lc -lgcc_s -lgcc"
  run_build psy_o2 "$run" "-std=c++20 -O2 -nostdinc++ -isystem $PSYCHICSTD" \
    LDFLAGS="-nodefaultlibs" LIBS="-lsupc++ -lm -lc -lgcc_s -lgcc"
done

# Read a timing file.
ms() { cat "${1}_r${2}_${3}.ms"; }

# Median of three integers.
median3() {
  awk "BEGIN {
    a[0]=$1; a[1]=$2; a[2]=$3
    if (a[0]>a[1]) { t=a[0]; a[0]=a[1]; a[1]=t }
    if (a[1]>a[2]) { t=a[1]; a[1]=a[2]; a[2]=t }
    if (a[0]>a[1]) { t=a[0]; a[0]=a[1]; a[1]=t }
    printf \"%d\", a[1]
  }"
}

# Median across the 3 runs for a given variant and phase.
med() { median3 "$(ms "$1" 1 "$2")" "$(ms "$1" 2 "$2")" "$(ms "$1" 3 "$2")"; }

SYS_O0_CONFIGURE=$(med sys_o0 configure)
SYS_O0_MAKE=$(med sys_o0 make)
SYS_O0_CHECK=$(med sys_o0 check)
SYS_O2_CONFIGURE=$(med sys_o2 configure)
SYS_O2_MAKE=$(med sys_o2 make)
SYS_O2_CHECK=$(med sys_o2 check)
PSY_O0_CONFIGURE=$(med psy_o0 configure)
PSY_O0_MAKE=$(med psy_o0 make)
PSY_O0_CHECK=$(med psy_o0 check)
PSY_O2_CONFIGURE=$(med psy_o2 configure)
PSY_O2_MAKE=$(med psy_o2 make)
PSY_O2_CHECK=$(med psy_o2 check)

rm -f sys_o0_r*.ms sys_o2_r*.ms psy_o0_r*.ms psy_o2_r*.ms

# Formatting helpers.
sec() { awk "BEGIN { printf \"%.2f\", $1 / 1000 }"; }
speedup() { awk "BEGIN { printf \"%.2fx\", $1 / ($2 == 0 ? 1 : $2) }"; }

table_row() {
  local label="$1" sys="$2" psy="$3"
  printf "| %s | %s | %s | %s |\n" \
    "$label" "$(sec "$sys")" "$(sec "$psy")" "$(speedup "$sys" "$psy")"
}

REPORT="$SCRIPT_DIR/rdfind_speed_report.md"
{
  printf "# rdfind compile speed test\n\n"
  printf "Median of 3 runs. Both builds use \`-std=c++20\`.\n"
  printf "Speedup = system time / psychicstd time (>1x means psychicstd is faster).\n"
  printf "make check measures runtime, not compile time.\n\n"

  printf "## Debug (-O0)\n\n"
  printf "| step | system (s) | psychicstd (s) | speedup |\n"
  printf "| --- | ---: | ---: | ---: |\n"
  table_row "configure" "$SYS_O0_CONFIGURE" "$PSY_O0_CONFIGURE"
  table_row "make" "$SYS_O0_MAKE" "$PSY_O0_MAKE"
  table_row "make check" "$SYS_O0_CHECK" "$PSY_O0_CHECK"

  printf "\n## Release (-O2)\n\n"
  printf "| step | system (s) | psychicstd (s) | speedup |\n"
  printf "| --- | ---: | ---: | ---: |\n"
  table_row "configure" "$SYS_O2_CONFIGURE" "$PSY_O2_CONFIGURE"
  table_row "make" "$SYS_O2_MAKE" "$PSY_O2_MAKE"
  table_row "make check" "$SYS_O2_CHECK" "$PSY_O2_CHECK"
} | tee "$REPORT"

mdformat "$REPORT"

if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
  cat "$REPORT" >>"$GITHUB_STEP_SUMMARY"
fi

echo "Report saved to $REPORT"
