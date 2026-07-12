#!/usr/bin/env python3
"""Real-world project build recipes for psychicstd.

Each recipe fetches a project and times its build phases -- configure, compile,
run tests -- under a given toolchain (compiler + flags). The perf-diff driver
(scripts/compare_realworld.py) runs a recipe with main's headers and the PR's
headers on one host to show how a change affects real build times.

Add a project by writing a `_name() -> Project` factory function -- it returns
the project's metadata (version, phases, comments) together with its
`build(toolchain) -> {phase: milliseconds}` closure -- and registering it in
PROJECTS. This is also intended to grow into the single home for the
real-world build recipes (currently spread across the test_*.sh scripts).
"""

import hashlib
import os
import subprocess
import tarfile
import tempfile
import time
import urllib.request
from collections.abc import Callable
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from pathlib import Path

RW_DIR = Path(__file__).resolve().parent
PHASES = ("configure", "compile", "run tests")


@dataclass(frozen=True)
class Toolchain:
    """A compiler invocation: flags plus the link additions psychicstd needs.

    build_type is "debug" or "release" -- an abstract choice each recipe
    translates into its own build system's convention (CMAKE_BUILD_TYPE, an
    -O flag, ...), since that mapping isn't the same across build systems.
    """

    cxx: str
    cxxflags: str
    ldflags: str = ""
    libs: str = ""
    build_type: str = "debug"
    enable_ccache: bool = False


@dataclass(frozen=True)
class Project:
    """A real-world project recipe: the pinned version under test and its
    `build(toolchain) -> {phase: milliseconds}` function.

    phases: the phase keys of `build()`'s return value to measure and report
    (a project may return extra keys it doesn't want reported). Defaults to
    PHASES, but a project can use its own names, e.g. fmt has no test suite to
    run and reports "example"/"run example" instead of "run tests".

    comment: optional project-level note. comments: optional per-phase note.
    Both empty by default.
    """

    version: str
    build: Callable[[Toolchain], dict[str, float]]
    phases: tuple[str, ...] = PHASES
    comment: str = ""
    comments: dict[str, str] = field(default_factory=dict)


def _run(cmd: list[str], cwd: Path, env: dict[str, str]) -> None:
    r = subprocess.run(
        cmd,
        cwd=cwd,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    if r.returncode != 0:
        print(f"failed command {cmd[0]}!\noutput:")
        print(r.stdout)
        r.check_returncode()


def _timed(cmd: list[str], cwd: Path, env: dict[str, str]) -> float:
    t0 = time.monotonic()
    _run(cmd, cwd, env)
    return (time.monotonic() - t0) * 1000.0


def _timed_many(cmds: list[list[str]], cwd: Path, env: dict[str, str]) -> float:
    """Wall-clock time to run many independent commands (e.g. compiling one
    file each) concurrently across CPUs -- unlike summing each command's own
    elapsed time, this reflects the phase's actual wall-clock duration."""
    t0 = time.monotonic()
    with ThreadPoolExecutor(max_workers=os.cpu_count() or 1) as pool:
        for _ in pool.map(lambda cmd: _run(cmd, cwd, env), cmds):
            pass
    return (time.monotonic() - t0) * 1000.0


def _fetch(url: str, dest: Path, sha256: str) -> None:
    if not dest.exists():
        urllib.request.urlretrieve(url, dest)
    got = hashlib.sha256(dest.read_bytes()).hexdigest()
    if got != sha256:
        raise SystemExit(f"sha256 mismatch for {dest.name}: {got}")


def _env(tc: Toolchain, **extra: str) -> dict[str, str]:
    """Base subprocess env for a recipe: disables ccache unless the toolchain
    opts in, since a warm cache would skew compile-time measurements."""
    env = {**os.environ, **extra}
    if not tc.enable_ccache:
        env["CCACHE_DISABLE"] = "1"
    return env


# --- catch2 -----


def _catch2() -> Project:
    version = "3.8.0"
    url = f"https://github.com/catchorg/Catch2/archive/refs/tags/v{version}.tar.gz"
    checksum = "1ab2de20460d4641553addfdfe6acd4109d871d5531f8f519a52ea4926303087"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"Catch2-v{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-catch2-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"Catch2-{version}"

            env = _env(tc)
            configure = [
                "cmake",
                "-B",
                "build",
                "--preset",
                "basic-tests",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + tc.cxx,
                "-DCMAKE_CXX_FLAGS=" + tc.cxxflags,
                "-DCMAKE_EXE_LINKER_FLAGS=" + tc.ldflags,
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + tc.libs,
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DCATCH_ENABLE_WERROR=OFF",
                "-DBUILD_SHARED_LIBS=OFF",
            ]
            jobs = f"-j{os.cpu_count() or 1}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(["cmake", "--build", "build", jobs], src, env),
                "run tests": _timed(
                    [
                        "ctest",
                        "--test-dir",
                        "build",
                        "--output-on-failure",
                        "-E",
                        "ApprovalTests",
                        jobs,
                    ],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        phases=("compile", "run tests"),
        comments={"run tests": "the approval tests are ignored"},
    )


# --- cppcheck -----------------------------------------------------------

_CPPCHECK_FILES = (
    "lib/addoninfo.cpp",
    "lib/analyzerinfo.cpp",
    "lib/color.cpp",
    "lib/timer.cpp",
    "lib/errortypes.cpp",
    "lib/astutils.cpp",
    "lib/checkassert.cpp",
    "lib/checkbool.cpp",
    "lib/checkcondition.cpp",
    "lib/checkfunctions.cpp",
    "lib/tokenize.cpp",
    "lib/symboldatabase.cpp",
    "lib/valueflow.cpp",
    "lib/checkclass.cpp",
    "lib/checkbufferoverrun.cpp",
    "cli/cmdlineparser.cpp",
    "cli/filelister.cpp",
    "cli/cppcheckexecutor.cpp",
)


def _cppcheck() -> Project:
    version = "2.21.0"
    url = f"https://github.com/danmar/cppcheck/archive/refs/tags/{version}.tar.gz"
    checksum = "f028ff75ca5372738f3737c8b3e8611426a6526b6aea2ef01301ab0f5902f044"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"cppcheck-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-cppcheck-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"cppcheck-{version}"

            env = _env(tc)
            cxxflags = [
                *tc.cxxflags.split(),
                "-I",
                str(src / "lib"),
                "-I",
                str(src / "externals" / "picojson"),
                "-I",
                str(src / "externals" / "simplecpp"),
                "-I",
                str(src / "externals" / "tinyxml2"),
                "-I",
                str(src / "cli"),
                "-I",
                str(src / "frontend"),
                '-DFILESDIR="/usr/local/share/Cppcheck"',
                "-DHAVE_EXECINFO_H=1",
            ]

            compile_ms = 0.0
            for name in _CPPCHECK_FILES:
                cpp = src / name
                obj = work / (Path(name).stem + ".o")
                cmd = [tc.cxx, *cxxflags, str(cpp), "-c", "-o", str(obj)]
                compile_ms += _timed(cmd, src, env)

            return {"compile": compile_ms}

    return Project(
        version=version,
        build=build,
        phases=("compile",),
        comment="a fixed subset of cppcheck's source files is compiled "
        "individually (no link/run step); times are summed. the real binary "
        "doesn't build: threadexecutor.cpp needs <future> (unimplemented) "
        "and stacktrace.cpp needs <cxxabi.h> (not shadowed by psychicstd).",
    )


# --- eigen ------------------------------------------------------------

_EIGEN_TEST_LIST = (
    "basicstuff",
    "meta",
    "numext",
    "block",
    "corners",
    "determinant",
    "diagonal",
    "array_cwise",
    "array_for_matrix",
    "constructor",
    "adjoint",
    "triangular",
)


def _eigen() -> Project:
    version = "3.4.0"
    url = (
        f"https://gitlab.com/libeigen/eigen/-/archive/{version}/eigen-{version}.tar.gz"
    )
    checksum = "8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"eigen-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-eigen-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"eigen-{version}"
            test_dir = src / "test"

            # main.h's FORBIDDEN_IDENTIFIER macros clash with psychicstd's names.
            main_h = test_dir / "main.h"
            text = main_h.read_text()
            for name in (
                "FORBIDDEN_IDENTIFIER",
                "B0 FORBIDDEN_IDENTIFIER",
                "I  FORBIDDEN_IDENTIFIER",
            ):
                text = text.replace(f"#define {name}", f"// DISABLED: #define {name}")
            main_h.write_text(text)

            env = _env(tc)
            cxxflags = [
                *tc.cxxflags.split(),
                "-I",
                str(src),
                "-I",
                str(test_dir),
                "-DEIGEN_TEST_MAX_SIZE=320",
            ]

            compile_ms = 0.0
            run_ms = 0.0
            for name in _EIGEN_TEST_LIST:
                cpp = test_dir / f"{name}.cpp"
                binary = work / f"eigen_{name}"
                cmd = (
                    [tc.cxx, *cxxflags, str(cpp)]
                    + (tc.ldflags.split() if tc.ldflags else [])
                    + (tc.libs.split() if tc.libs else [])
                    + ["-o", str(binary)]
                )
                compile_ms += _timed(cmd, src, env)
                run_ms += _timed([str(binary)], src, env)

            return {"compile": compile_ms, "run tests": run_ms}

    return Project(
        version=version,
        build=build,
        phases=("compile", "run tests"),
        comment="eigen has no configure step; a fixed subset of its test "
        "suite is compiled and run individually, with times summed.",
    )


# --- fmt --------------------------------------------------------------


def _fmt() -> Project:
    version = "11.1.4"
    url = f"https://github.com/fmtlib/fmt/archive/refs/tags/{version}.tar.gz"
    checksum = "ac366b7b4c2e9f0dde63a59b3feb5ee59b67974b14ee5dc9ea8ad78aa2c1ee1e"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"fmt-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-fmt-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"fmt-{version}"

            env = _env(tc)
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + tc.cxx,
                "-DCMAKE_CXX_FLAGS=" + tc.cxxflags,
                "-DCMAKE_EXE_LINKER_FLAGS=" + tc.ldflags,
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + tc.libs,
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DFMT_DOC=OFF",
                "-DFMT_TEST=ON",
                "-DFMT_INSTALL=OFF",
                "-DBUILD_SHARED_LIBS=OFF",
            ]
            jobs = f"-j{os.cpu_count() or 1}"
            configure_ms = _timed(configure, src, env)
            compile_ms = _timed(["cmake", "--build", "build", jobs], src, env)

            run_tests_ms = _timed(
                [
                    "ctest",
                    "--test-dir",
                    "build",
                    "--output-on-failure",
                    "-j",
                    str(os.cpu_count() or 1),
                ],
                src,
                env,
            )

            return {
                "configure": configure_ms,
                "compile": compile_ms,
                "run tests": run_tests_ms,
            }

    return Project(
        version=version,
        build=build,
        phases=("compile", "run tests"),
        comment="fmt is built with locale support; its own unit tests are run.",
    )


# --- nlohmann json -----------------------------------------------------

# unicode|cbor|msgpack: long-running (tens of seconds each) -- revisit later.
# algorithms: one partial_sort assertion checks the tail order, which the
# standard leaves unspecified (psychicstd's partial_sort is a full sort).
# cmake_fetch: exercises FetchContent, not psychicstd.
# cmake_import: upstream bug -- cmake_import(_minver)_configure/build add_test()
# without WORKING_DIRECTORY, so both pairs default to the same build/tests dir
# and race under -j; upstream already labels them "not_reproducible".
_NLOHMANN_TEST_EXCLUDE = "unicode|cbor|msgpack|algorithms|cmake_fetch|cmake_import"


def _nlohmann() -> Project:
    version = "3.12.0"
    url = f"https://github.com/nlohmann/json/archive/refs/tags/v{version}.tar.gz"
    checksum = "4b92eb0c06d10683f7447ce9406cb97cd4b453be18d7279320f7b2f025c10187"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"nlohmann-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-nlohmann-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"json-{version}"

            env = _env(tc)
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + tc.cxx,
                "-DCMAKE_CXX_FLAGS=" + tc.cxxflags,
                "-DCMAKE_EXE_LINKER_FLAGS=" + tc.ldflags,
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + tc.libs,
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DJSON_TestStandards=20",
                "-DJSON_BuildTests=ON",
            ]
            jobs = f"-j{os.cpu_count() or 1}"
            configure_ms = _timed(configure, src, env)
            compile_ms = _timed(["cmake", "--build", "build", jobs], src, env)
            run_tests_ms = _timed(
                [
                    "ctest",
                    "--test-dir",
                    "build",
                    "--output-on-failure",
                    "-E",
                    _NLOHMANN_TEST_EXCLUDE,
                    jobs,
                ],
                src,
                env,
            )

            # Compile (but don't run) the 217 documented API examples.
            include_dir = src / "include"
            example_cmds = []
            for cpp in sorted(
                (src / "docs" / "mkdocs" / "docs" / "examples").glob("*.cpp")
            ):
                binary = work / (cpp.stem + "_bin")
                example_cmds.append(
                    [tc.cxx, *tc.cxxflags.split(), "-I", str(include_dir), str(cpp)]
                    + (tc.ldflags.split() if tc.ldflags else [])
                    + (tc.libs.split() if tc.libs else [])
                    + ["-o", str(binary)]
                )
            examples_ms = _timed_many(example_cmds, src, env)

            return {
                "configure": configure_ms,
                "compile": compile_ms,
                "run tests": run_tests_ms,
                "examples": examples_ms,
            }

    return Project(
        version=version,
        build=build,
        phases=("configure", "compile", "run tests", "examples"),
        comments={
            "run tests": "unicode/cbor/msgpack (slow), algorithms (unspecified "
            "tail order), cmake_fetch/cmake_import (not applicable) excluded",
            "examples": "217 documented API examples, compiled but not run",
        },
    )


# --- rdfind ---------------------------------------------------------------


_RDFIND_COMMIT = "787b01ab378c70cb6bb3ef5166525f3ff8939a23"
# rdfind's autoconf build has no build-type concept -- an -O flag is its equivalent.
_RDFIND_OPT_FLAG = {"debug": "-O0", "release": "-O2"}


def _rdfind() -> Project:
    fromcommit = True

    if fromcommit:
        # from a certain commit hash
        psychicstrictlevel = 0
        commithash = _RDFIND_COMMIT
        url = f"https://github.com/pauldreik/rdfind/archive/{commithash}.tar.gz"
        checksum = "057ae066b2f7349cb84e4b48ab3ab897d88afc3005bd6d8292c95fa012467659"
        tarball_name = f"rdfind-{commithash}.tar.gz"
        src_name = f"rdfind-{commithash}"
        proj_version = f"commit {commithash[:12]}"
    else:
        # from a release
        psychicstrictlevel = 2
        version = "1.8.0"
        url = (
            "https://github.com/pauldreik/rdfind/releases/download/"
            f"releases%2F{version}/rdfind-{version}.tar.gz"
        )
        checksum = "0a2d0d32002cc2dc0134ee7b649bcc811ecfb2f8d9f672aa476a851152e7af35"
        tarball_name = f"rdfind-{version}.tar.gz"
        src_name = f"rdfind-{version}"
        proj_version = version

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / tarball_name
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-rdfind-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / src_name
            env = _env(
                tc,
                CXX=tc.cxx,
                CXXFLAGS=tc.cxxflags
                + f" {_RDFIND_OPT_FLAG[tc.build_type]}"
                + f" -D_PSYCHICSTD_COMPATIBILITY_LEVEL={psychicstrictlevel}",
            )
            if fromcommit:
                _timed(["./bootstrap.sh"], src, env)
            configure = ["./configure"]
            if tc.ldflags:
                configure.append(f"LDFLAGS={tc.ldflags}")
            if tc.libs:
                configure.append(f"LIBS={tc.libs}")
            jobs = f"-j{os.cpu_count() or 1}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(["make", jobs], src, env),
                "run tests": _timed(["make", "check"], src, env),
            }

    return Project(
        version=proj_version,
        build=build,
        comment="rdfind is an autoconf based project. It uses psychic strict mode.",
    )


# --- simdutf -----


def _simdutf() -> Project:
    version = "9.0.0"
    url = f"https://github.com/simdutf/simdutf/archive/refs/tags/v{version}.tar.gz"
    checksum = "fd2ce975f29809a975a8da8843cfb3a7265af3f71be548f199d23cf65e101764"
    psychicstrictlevel = 0

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"simdutf-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-simdutf-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"simdutf-{version}"

            env = _env(tc)
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build-with-psychic",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DSIMDUTF_FAST_TESTS=On",
                "-DSIMDUTF_TOOLS=On",
                "-DCMAKE_CXX_COMPILER=" + tc.cxx,
                "-DSIMDUTF_CXX_STANDARD=20",
                "-DCMAKE_CXX_FLAGS="
                + tc.cxxflags
                + f" -D_PSYCHICSTD_COMPATIBILITY_LEVEL={psychicstrictlevel}",
                "-DCMAKE_EXE_LINKER_FLAGS=" + tc.ldflags,
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + tc.libs,
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DBUILD_SHARED_LIBS=OFF",
            ]
            jobs = f"-j{os.cpu_count() or 1}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(
                    ["cmake", "--build", "build-with-psychic", jobs], src, env
                ),
                "run tests": _timed(
                    [
                        "ctest",
                        "--test-dir",
                        "build-with-psychic",
                        "--output-on-failure",
                        jobs,
                    ],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        phases=("compile", "run tests"),
        comment="simdutf code is mostly simd intrinsics.",
    )


PROJECTS: dict[str, Project] = {
    "catch2": _catch2(),
    "cppcheck": _cppcheck(),
    "eigen": _eigen(),
    "fmt": _fmt(),
    "nlohmann": _nlohmann(),
    "rdfind": _rdfind(),
    "simdutf": _simdutf(),
}
