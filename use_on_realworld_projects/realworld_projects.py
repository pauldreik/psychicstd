#!/usr/bin/env python3
"""Real-world project build recipes for psychicstd.

Each recipe fetches a project and times its build phases -- configure, compile,
tests -- under a given toolchain (compiler + flags). The perf-diff driver
(scripts/compare_realworld.py) runs a recipe with main's headers and the PR's
headers on one host to show how a change affects real build times.

Add a project by writing a `build(toolchain) -> {phase: milliseconds}` function
and registering it in PROJECTS. This is also intended to grow into the single
home for the real-world build recipes (currently spread across the test_*.sh
scripts).
"""

import hashlib
import os
import shutil
import subprocess
import tarfile
import tempfile
import time
import urllib.request
from pathlib import Path

RW_DIR = Path(__file__).resolve().parent
PHASES = ("configure", "compile", "tests")


class Toolchain:
    """A compiler invocation: flags plus the link additions psychicstd needs."""

    def __init__(self, cxx, cxxflags, ldflags="", libs=""):
        self.cxx = cxx
        self.cxxflags = cxxflags
        self.ldflags = ldflags
        self.libs = libs


def _timed(cmd, cwd, env):
    t0 = time.monotonic()
    r = subprocess.run(
        cmd,
        cwd=cwd,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if r.returncode != 0:
        print(f"failed command {cmd[0]}!\noutput:")
        print(r.stdout.decode("utf-8"))
        r.check_returncode()

    return (time.monotonic() - t0) * 1000.0


def _fetch(url, dest, sha256):
    if not dest.exists():
        urllib.request.urlretrieve(url, dest)
    got = hashlib.sha256(dest.read_bytes()).hexdigest()
    if got != sha256:
        raise SystemExit(f"sha256 mismatch for {dest.name}: {got}")


# --- simdutf -----


def _simdutf(tc):

    version = "9.0.0"
    url = f"https://github.com/simdutf/simdutf/archive/refs/tags/v{version}.tar.gz"
    checksum = "fd2ce975f29809a975a8da8843cfb3a7265af3f71be548f199d23cf65e101764"
    psychicstrictlevel = 0
    tarball = RW_DIR / f"simdutf-{version}.tar.gz"
    _fetch(url, tarball, checksum)

    work = Path(tempfile.mkdtemp(prefix="rw-simdutf-"))
    try:
        with tarfile.open(tarball) as t:
            t.extractall(work)
        src = work / f"simdutf-{version}"

        env = {
            **os.environ,
            "CCACHE_DISABLE": "1",
        }
        configure = [
            "cmake",
            "-S",
            ".",
            "-B",
            "build-with-psychic",
            "-GNinja",
            "-DCMAKE_BUILD_TYPE=Debug",
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
            "tests": _timed(
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
    finally:
        shutil.rmtree(work, ignore_errors=True)


# --- rdfind ---------------------------------------------------------------


def _rdfind(tc):

    fromcommit = True

    if fromcommit:
        # from a certain commit hash
        psychicstrictlevel = 0
        commithash = "787b01ab378c70cb6bb3ef5166525f3ff8939a23"
        tarball = RW_DIR / f"rdfind-{commithash}.tar.gz"
        _fetch(
            f"https://github.com/pauldreik/rdfind/archive/{commithash}.tar.gz",
            tarball,
            "057ae066b2f7349cb84e4b48ab3ab897d88afc3005bd6d8292c95fa012467659",
        )
    else:
        # from a release
        psychicstrictlevel = 2
        version = "1.8.0"
        tarball = RW_DIR / f"rdfind-{version}.tar.gz"
        _fetch(
            f"https://github.com/pauldreik/rdfind/releases/download/releases%2F{version}/rdfind-{version}.tar.gz",
            tarball,
            "0a2d0d32002cc2dc0134ee7b649bcc811ecfb2f8d9f672aa476a851152e7af35",
        )

    work = Path(tempfile.mkdtemp(prefix="rw-rdfind-"))
    try:
        with tarfile.open(tarball) as t:
            t.extractall(work)
        if fromcommit:
            src = work / f"rdfind-{commithash}"
        else:
            src = work / f"rdfind-{version}"
        env = {
            **os.environ,
            "CXX": tc.cxx,
            "CXXFLAGS": tc.cxxflags
            + f" -D_PSYCHICSTD_COMPATIBILITY_LEVEL={psychicstrictlevel}",
            "CCACHE_DISABLE": "1",
        }
        if fromcommit:
            _timed(
                [
                    "./bootstrap.sh",
                ],
                src,
                env,
            )
        configure = ["./configure"]
        if tc.ldflags:
            configure.append(f"LDFLAGS={tc.ldflags}")
        if tc.libs:
            configure.append(f"LIBS={tc.libs}")
        jobs = f"-j{os.cpu_count() or 1}"
        return {
            "configure": _timed(configure, src, env),
            "compile": _timed(["make", jobs], src, env),
            "tests": _timed(["make", "check"], src, env),
        }
    finally:
        shutil.rmtree(work, ignore_errors=True)


PROJECTS = {
    "rdfind": _rdfind,
    "simdutf": _simdutf,
}
