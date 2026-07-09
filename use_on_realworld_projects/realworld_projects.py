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

    phases: which of PHASES are meaningful to report for this project (the
    recipe still runs and times all of them internally; this only controls
    what a report shows). E.g. a CMake project's "configure" is often mostly
    download/CMake overhead unrelated to psychicstd, while for an autoconf
    project like rdfind it's a real, relevant measurement.

    comment: optional project-level note (e.g. what build system it uses, or
    what psychicstd compatibility level it's tested at). Empty by default.

    comments: optional per-phase caveat shown alongside the numbers (e.g. "some
    tests are excluded"). Empty by default.
    """

    version: str
    build: Callable[[Toolchain], dict[str, float]]
    phases: tuple[str, ...] = PHASES
    comment: str = ""
    comments: dict[str, str] = field(default_factory=dict)


def _timed(cmd: list[str], cwd: Path, env: dict[str, str]) -> float:
    t0 = time.monotonic()
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
        comments={
            "run tests": "the approval tests are ignored"
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
                "-DSIMDUTF_TOOLS=Off",  # sutf/fastbase64 need <filesystem>, unsupported
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
        comment="simdutf code is mostly simd intrinsics. builds without tools because of filesystem not being available yet.",
    )


PROJECTS: dict[str, Project] = {
    "catch2": _catch2(),
    "rdfind": _rdfind(),
    "simdutf": _simdutf(),
}
