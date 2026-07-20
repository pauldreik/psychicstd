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
import shlex
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


def _jobs() -> str:
    """Cap parallel builds so each active compiler gets about 1.5 GiB."""
    available = (
        os.sysconf("SC_PAGE_SIZE") * os.sysconf("SC_AVPHYS_PAGES")
        if hasattr(os, "sysconf")
        else 0
    )
    memory_jobs = available // (1536 * 1024 * 1024) if available else 1
    return f"-j{max(1, min(os.cpu_count() or 1, memory_jobs))}"


def _compiler_wrapper(
    path: Path, tc: Toolchain, extra_cxxflags: tuple[str, ...] = ()
) -> Path:
    """Write a compiler wrapper that keeps toolchain flags last.

    Some projects add their own -std flag after CMAKE_CXX_FLAGS.  psychicstd
    requires C++20, so a wrapper is the simplest way to retain the last word.
    Link-only additions are kept off compile and preprocess invocations.
    """
    compiler = shlex.split(tc.cxx)
    cxxflags = [*shlex.split(tc.cxxflags), *extra_cxxflags]
    link_flags = shlex.split(tc.ldflags) + shlex.split(tc.libs)
    path.write_text(
        "#!/usr/bin/env python3\n"
        "import os\n"
        "import sys\n\n"
        f"compiler = {compiler!r}\n"
        f"cxxflags = {cxxflags!r}\n"
        f"link_flags = {link_flags!r}\n"
        "args = sys.argv[1:]\n"
        "linking = not any(flag in args for flag in ('-c', '-E', '-S'))\n"
        "argv = compiler + args + cxxflags\n"
        "if linking:\n"
        "    argv += link_flags\n"
        "os.execvp(compiler[0], argv)\n"
    )
    path.chmod(0o755)
    return path


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
            jobs = _jobs()
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


# --- abseil ------------------------------------------------------------


def _abseil() -> Project:
    version = "20260107.1"
    url = f"https://github.com/abseil/abseil-cpp/archive/refs/tags/{version}.tar.gz"
    checksum = "4314e2a7cbac89cac25a2f2322870f343d81579756ceff7f431803c2c9090195"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"abseil-cpp-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-abseil-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"abseil-cpp-{version}"

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
                "-DCMAKE_CXX_STANDARD=20",
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DABSL_BUILD_TESTING=ON",
                "-DABSL_USE_GOOGLETEST_HEAD=ON",
                "-DBUILD_SHARED_LIBS=OFF",
            ]
            jobs = _jobs()
            base_targets = [
                "base",
                "raw_logging_internal",
                "spinlock_wait",
                "malloc_internal",
                "throw_delegate",
                "scoped_set_env",
                "strerror",
                "tracing_internal",
            ]
            base_targets += [
                "absl_atomic_hook_test",
                "absl_attributes_test",
                "absl_bit_cast_test",
                "absl_casts_test",
                "absl_errno_saver_test",
                "absl_throw_delegate_test",
                "absl_endian_test",
                "absl_no_destructor_test",
            ]
            base_tests = [
                target for target in base_targets if target.startswith("absl_")
            ]
            test_filter = "^(" + "|".join(base_tests) + ")$"

            def compile_base() -> float:
                t0 = time.monotonic()
                for target in base_targets:
                    _run(["ninja", "-C", "build", target, jobs], src, env)
                return (time.monotonic() - t0) * 1000.0

            return {
                "configure": _timed(configure, src, env),
                "compile": compile_base(),
                "run tests": _timed(
                    [
                        "ctest",
                        "--test-dir",
                        "build",
                        "--output-on-failure",
                        "-R",
                        test_filter,
                        jobs,
                    ],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        comment="Builds and runs Abseil's upstream absl/base tests.",
    )


# --- googletest --------------------------------------------------------


def _googletest() -> Project:
    version = "1.16.0"
    url = f"https://github.com/google/googletest/archive/refs/tags/v{version}.tar.gz"
    checksum = "78c676fc63881529bf97bf9d45948d905a66833fbfa5318ea2cd7478cb98f399"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"googletest-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-googletest-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"googletest-{version}"
            # Point GoogleTest at psychicstd's ABI declaration without
            # changing the compiler's RTTI or exception settings.
            wrapper = _compiler_wrapper(work / "cxx", tc, ("-DGTEST_HAS_CXXABI_H_=1",))
            env = _env(tc)
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + str(wrapper),
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-Dgtest_build_tests=ON",
                "-Dgtest_build_samples=OFF",
                "-DBUILD_GMOCK=OFF",
                "-DINSTALL_GTEST=OFF",
            ]
            jobs = f"-j{os.cpu_count() or 1}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(["cmake", "--build", "build", jobs], src, env),
                "run tests": _timed(
                    ["ctest", "--test-dir", "build", "--output-on-failure", jobs],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        comment="Builds GoogleTest's upstream unit tests with GMock and samples "
        "disabled, then runs the resulting CTest suite.",
    )


# --- cmake -------------------------------------------------------------


def _cmake() -> Project:
    version = "4.3.4"
    url = f"https://cmake.org/files/v4.3/cmake-{version}.tar.gz"
    checksum = "fdeff897b9eb49d764539f2b1edc6eb7e1440df325678a97c1978499e931adda"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"cmake-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-cmake-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"cmake-{version}"
            # CMake relies on ADL finding these through its string iterator,
            # which is not guaranteed when an implementation uses pointers.
            compat_header = work / "cmake-compat.h"
            compat_header.write_text(
                "#include <algorithm>\nusing std::find;\nusing std::find_if_not;\n"
            )
            wrapper = _compiler_wrapper(
                work / "cxx", tc, ("-include", str(compat_header))
            )

            env = _env(tc)
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + str(wrapper),
                "-DCMAKE_CXX_STANDARD=20",
                "-DCMAKE_USE_OPENSSL=OFF",
                "-DCMake_ENABLE_DEBUGGER=OFF",
                "-DBUILD_TESTING=ON",
            ]
            jobs = f"-j{os.cpu_count() or 1}"
            configure_ms = _timed(configure, src, env)
            compile_ms = _timed(
                [
                    "cmake",
                    "--build",
                    "build",
                    "--target",
                    "cmsysTestsCxx",
                    "testEncoding",
                    "cmjsoncpp",
                    "cmstd",
                    "CMakeLib",
                    jobs,
                ],
                src,
                env,
            )
            run_tests_ms = _timed(
                [
                    "ctest",
                    "--test-dir",
                    "build",
                    "--output-on-failure",
                    "-R",
                    r"^kwsys\.test(Configure|Status|SystemTools|"
                    r"CommandLineArguments1?|Directory|Encoding|"
                    r"SystemInformation)$",
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
        comment="Builds upstream CMake's core static library together with "
        "its KWSys, std-compatibility, and JSON support targets, then runs "
        "the supported KWSys tests. OpenSSL and debugger support are disabled.",
    )


# --- cppcheck -----------------------------------------------------------


def _cppcheck() -> Project:
    version = "2.21.0"
    url = f"https://github.com/cppcheck-opensource/cppcheck/archive/refs/tags/{version}.tar.gz"
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
            # The Makefile appends -std=c++11 to CXXFLAGS. Keep psychicstd's
            # required C++20 flags last with a wrapper, as other recipes do.
            wrapper = _compiler_wrapper(work / "cxx", tc)
            cppflags = " ".join(
                (
                    "-Ilib",
                    "-Ifrontend",
                    "-Icli",
                    "-isystem externals",
                    "-isystem externals/picojson",
                    "-isystem externals/simplecpp",
                    "-isystem externals/tinyxml2",
                    "-DHAVE_EXECINFO_H=1",
                )
            )
            jobs = f"-j{os.cpu_count() or 1}"
            make_args = [
                jobs,
                "CXX=" + str(wrapper),
                "CPPFLAGS=" + cppflags,
                "CXXFLAGS=",
                "FILESDIR=/usr/local/share/Cppcheck",
                "LDFLAGS=" + tc.ldflags,
                "LIBS=" + tc.libs,
            ]
            return {
                "compile": _timed(["make", "all", *make_args], src, env),
                # This test hard-codes libstdc++'s vector::at diagnostic;
                # psychicstd intentionally does not promise that wording.
                "run tests": _timed(
                    [
                        "./testrunner",
                        "-x",
                        "TestSymbolDatabase::getVariableFromVarIdBoundsCheck",
                    ],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        phases=("compile", "run tests"),
        comment="the complete native Makefile build is compiled and linked; "
        "Cppcheck's own test runner is run with one libstdc++ diagnostic "
        "wording test excluded.",
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
            # fmt's tests specialize std::is_floating_point (UB but benign);
            # Apple clang 21's libc++ hard-errors on that, so the SYSTEM
            # baseline cannot build without disabling the diagnostic. No-op
            # for psychicstd, which does not restrict specialization.
            cxxflags = tc.cxxflags
            if os.uname().sysname == "Darwin":
                cxxflags += " -Wno-invalid-specialization"
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + tc.cxx,
                "-DCMAKE_CXX_FLAGS=" + cxxflags,
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


# --- rapidjson ---------------------------------------------------------


def _rapidjson() -> Project:
    commit = "24b5e7a8b27f42fa16b96fc70aade9106cf7102f"
    version = f"master-{commit[:12]}"
    url = f"https://github.com/Tencent/rapidjson/archive/{commit}.tar.gz"
    checksum = "2d2601a82d2d3b7e143a3c8d43ef616671391034bc46891a9816b79cf2d3e7a8"
    gtest_version = "1.8.0"
    gtest_url = (
        "https://github.com/google/googletest/archive/refs/tags/"
        f"release-{gtest_version}.tar.gz"
    )
    gtest_checksum = "58a6f4277ca2bc8565222b3bbd58a177609e9c488e8a72649359ba51450db7d8"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"rapidjson-{commit}.tar.gz"
        _fetch(url, tarball, checksum)
        gtest_tarball = RW_DIR / f"googletest-release-{gtest_version}.tar.gz"
        _fetch(gtest_url, gtest_tarball, gtest_checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-rapidjson-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            with tarfile.open(gtest_tarball) as t:
                t.extractall(work)
            src = work / f"rapidjson-{commit}"
            gtest_source = work / f"googletest-release-{gtest_version}" / "googletest"

            env = _env(tc)
            # GoogleTest 1.8.0 triggers this warning in modern GCC; RapidJSON
            # applies -Werror globally to its bundled test dependencies.
            cxxflags = tc.cxxflags + (
                " -Wno-error=maybe-uninitialized"
                " -Wno-error=sign-conversion -Wno-error=sign-compare"
            )
            if "clang" not in tc.cxx.lower():
                # GCC 12 diagnoses RapidJSON's realloc wrapper at -O3 as an
                # impossibly large allocation; its tests intentionally pass
                # the allocation through that wrapper.
                cxxflags += (
                    " -Wno-error=alloc-size-larger-than= -Wno-error=array-bounds"
                )
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + tc.cxx,
                "-DCMAKE_CXX_FLAGS=" + cxxflags,
                "-DCMAKE_EXE_LINKER_FLAGS=" + tc.ldflags,
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + tc.libs,
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
                "-DGTEST_SOURCE_DIR=" + str(gtest_source),
                "-DCMAKE_CXX_STANDARD=20",
                "-DRAPIDJSON_BUILD_CXX11=OFF",
                "-DRAPIDJSON_BUILD_CXX20=ON",
                "-DRAPIDJSON_BUILD_DOC=OFF",
                "-DRAPIDJSON_BUILD_EXAMPLES=ON",
                "-DRAPIDJSON_BUILD_TESTS=ON",
                "-DRAPIDJSON_ENABLE_INSTRUMENTATION_OPT=OFF",
            ]
            jobs = _jobs()
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(
                    [
                        "cmake",
                        "--build",
                        "build",
                        "--target",
                        "examples",
                        "archivertest",
                        "unittest",
                        jobs,
                    ],
                    src,
                    env,
                ),
                "run example": _timed(
                    [
                        str(src / "build" / "bin" / "simpledom"),
                    ],
                    src,
                    env,
                ),
                "run tests": _timed(
                    [str(src / "build" / "bin" / "unittest")],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        phases=("configure", "compile", "run example", "run tests"),
        comment="RapidJSON's examples, archivertest, and unit tests are built; "
        "simpledom and unittest are run.",
    )


# --- rdfind ---------------------------------------------------------------


_RDFIND_COMMIT = "787b01ab378c70cb6bb3ef5166525f3ff8939a23"
# rdfind's autoconf build has no build-type concept -- an -O flag is its equivalent.
_RDFIND_OPT_FLAG = {"debug": "-O0", "release": "-O2"}


def _rdfind() -> Project:
    fromcommit = True

    if fromcommit:
        # from a certain commit hash
        psychicstrictlevel = -0
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
    version = "master260712"  #
    commithash = "c04f2db9eee13fbd1b6dd1c2b2fb52374738dd4d"
    url = f"https://github.com/simdutf/simdutf/archive/{commithash}.tar.gz"
    checksum = "face6b2056da68df9d758ce0ac4cca91df90ab1c668512fa541eba4cd6668686"
    # DROPIN: tests/helpers/random_utf32.cpp calls ::abort without including
    # <cstdlib> (an upstream include-what-you-use bug; we never patch pinned
    # third-party sources), so strict mode cannot compile the test helpers.
    psychicstrictlevel = 2

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"simdutf-{commithash}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-simdutf-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"simdutf-{commithash}"

            env = _env(tc)
            # Darwin: CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY (below)
            # skips the link step of try_compile, so FindIconv misdetects
            # Darwin's separate libiconv as built into libc and the sutf tool
            # fails to link (undefined _iconv*) -- with the system stdlib too,
            # not just psychicstd. Link iconv explicitly.
            libs = tc.libs
            if os.uname().sysname == "Darwin":
                libs += " -liconv"
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
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + libs,
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


# --- boost asio ----------------------------------------------------------


def _boost_asio() -> Project:
    version = "1.91.0"
    url = f"https://archives.boost.io/release/{version}/source/boost_1_91_0.tar.gz"
    checksum = "5734305f40a76c30f951c9abd409a45a2a19fb546efe4162119250bbe4d3a463"

    def build(tc: Toolchain) -> dict[str, float]:
        # The upstream test Jamfile expects the normal Boost source tree and
        # its modular dependencies. The build below still selects only Asio's
        # tests, not the rest of Boost.
        tarball = RW_DIR / f"boost_{version.replace('.', '_')}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-boost-asio-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"boost_{version.replace('.', '_')}"

            env = _env(
                tc,
                CXX=tc.cxx,
                CXXFLAGS=tc.cxxflags,
            )
            configure_ms = _timed(["./bootstrap.sh"], src, env)
            test_dir = src / "libs" / "asio" / "test"
            # The upstream test Jamfile declares Boost.Regex, Context, and
            # Chrono as project-wide requirements, so even one Asio test builds
            # those unrelated libraries. Compile a small set of upstream test
            # translation units directly to keep this recipe about Asio.
            asio_tests = ["io_context.cpp", "steady_timer.cpp", "buffer.cpp"]
            build_dir = work / "asio-test-build"
            build_dir.mkdir()
            common = [
                tc.cxx,
                *shlex.split(tc.cxxflags),
                "-I",
                str(src),
                "-I",
                str(src / "libs" / "asio" / "include"),
                "-DBOOST_ALL_NO_LIB=1",
                "-DBOOST_ASIO_DISABLE_DEPRECATED_MSG=1",
                "-DBOOST_NO_AUTO_PTR=1",
                "-D_GNU_SOURCE=1",
                "-D_XOPEN_SOURCE=600",
                "-pthread",
            ]
            commands = [
                [
                    *common,
                    str(test_dir / name),
                    "-o",
                    str(build_dir / Path(name).stem),
                    *shlex.split(tc.ldflags),
                    *shlex.split(tc.libs),
                ]
                for name in asio_tests
            ]
            t0 = time.monotonic()
            for command in commands:
                _run(command, test_dir, env)
            compile_ms = (time.monotonic() - t0) * 1000.0
            t0 = time.monotonic()
            for name in asio_tests:
                _run([str(build_dir / Path(name).stem)], test_dir, env)
            run_ms = (time.monotonic() - t0) * 1000.0
            return {
                "configure": configure_ms,
                "compile": compile_ms,
                "run tests": run_ms,
            }

    return Project(
        version=version,
        build=build,
        comments={
            "compile": "Representative upstream Asio tests are compiled and "
            "linked directly; unrelated Boost libraries are excluded.",
        },
    )


# --- opencv --------------------------------------------------------------


def _opencv() -> Project:
    version = "4.13.0"
    url = f"https://github.com/opencv/opencv/archive/refs/tags/{version}.tar.gz"
    checksum = "1d40ca017ea51c533cf9fd5cbde5b5fe7ae248291ddf2af99d4c17cf8e13017d"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"opencv-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-opencv-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"opencv-{version}"

            env = _env(tc)
            # OpenCV enables a large collection of optional codecs, language
            # bindings, and hardware backends by default. They are unrelated
            # to the standard-library-heavy core and make this recipe depend
            # on whichever packages happen to be installed on the runner.
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
                "-DCMAKE_CXX_STANDARD=20",
                # The test support module owns the generated test targets;
                # without it BUILD_TESTS=ON leaves only the aggregate target.
                "-DBUILD_LIST=core,imgproc,ts",
                "-DBUILD_TESTS=ON",
                "-DBUILD_PERF_TESTS=OFF",
                "-DBUILD_EXAMPLES=OFF",
                "-DBUILD_opencv_apps=OFF",
                "-DBUILD_opencv_gapi=OFF",
                "-DBUILD_opencv_python3=OFF",
                "-DBUILD_JAVA=OFF",
                "-DWITH_IPP=OFF",
                "-DWITH_OPENCL=OFF",
                "-DWITH_OPENGL=OFF",
                "-DWITH_TBB=OFF",
                "-DWITH_GTK=OFF",
                "-DWITH_QT=OFF",
                "-DWITH_FFMPEG=OFF",
                "-DWITH_GSTREAMER=OFF",
                "-DWITH_V4L=OFF",
            ]
            jobs = _jobs()
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(
                    [
                        "cmake",
                        "--build",
                        "build",
                        "--target",
                        "opencv_test_core",
                        "opencv_test_imgproc",
                        jobs,
                    ],
                    src,
                    env,
                ),
                "run tests": _timed(
                    [
                        "ctest",
                        "--test-dir",
                        "build",
                        "--output-on-failure",
                        "-R",
                        r"^opencv_test_(core|imgproc)$",
                        jobs,
                    ],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        comment="Builds OpenCV's core and imgproc modules and runs their "
        "upstream tests; optional codecs, bindings, and hardware backends "
        "are disabled.",
    )


PROJECTS: dict[str, Project] = {
    "abseil": _abseil(),
    "boost-asio": _boost_asio(),
    "catch2": _catch2(),
    "cmake": _cmake(),
    "cppcheck": _cppcheck(),
    "eigen": _eigen(),
    "fmt": _fmt(),
    "googletest": _googletest(),
    "nlohmann": _nlohmann(),
    "opencv": _opencv(),
    "rapidjson": _rapidjson(),
    "rdfind": _rdfind(),
    "simdutf": _simdutf(),
}
