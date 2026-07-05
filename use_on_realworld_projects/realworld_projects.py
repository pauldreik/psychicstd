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
    subprocess.run(
        cmd,
        cwd=cwd,
        env=env,
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    return (time.monotonic() - t0) * 1000.0


def _fetch(url, dest, sha256):
    if not dest.exists():
        urllib.request.urlretrieve(url, dest)
    got = hashlib.sha256(dest.read_bytes()).hexdigest()
    if got != sha256:
        raise SystemExit(f"sha256 mismatch for {dest.name}: {got}")


# --- rdfind ---------------------------------------------------------------


def _rdfind(tc):
    tarball = RW_DIR / "rdfind-1.8.0.tar.gz"
    _fetch(
        "https://github.com/pauldreik/rdfind/releases/download/releases%2F1.8.0/rdfind-1.8.0.tar.gz",
        tarball,
        "0a2d0d32002cc2dc0134ee7b649bcc811ecfb2f8d9f672aa476a851152e7af35",
    )
    work = Path(tempfile.mkdtemp(prefix="rw-rdfind-"))
    try:
        with tarfile.open(tarball) as t:
            t.extractall(work)
        src = work / "rdfind-1.8.0"
        env = {
            **os.environ,
            "CXX": tc.cxx,
            "CXXFLAGS": tc.cxxflags,
            "CCACHE_DISABLE": "1",
        }
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
}
