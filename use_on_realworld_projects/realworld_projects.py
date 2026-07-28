#!/usr/bin/env python3
"""Real-world project build recipes for psychicstd.

Each recipe fetches a project and times its build phases -- configure, compile,
run tests -- under a given toolchain (compiler + flags). The perf-diff driver
(scripts/compare_realworld_to_ref.py) runs a recipe with main's headers and the PR's
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
import shutil
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
class Parallelism:
    logical_cpus: int
    jobs: int
    memory_jobs: int
    available_memory: int

    @property
    def memory_limited(self) -> bool:
        return self.jobs < self.logical_cpus


def detect_parallelism() -> Parallelism:
    """Choose one shared job count for every real-world recipe."""
    # SC_AVPHYS_PAGES excludes reclaimable cache and substantially
    # under-reports usable memory on Linux. MemAvailable is the kernel's
    # estimate of memory available without swapping.
    available = 0
    try:
        for line in Path("/proc/meminfo").read_text().splitlines():
            if line.startswith("MemAvailable:"):
                available = int(line.split()[1]) * 1024
                break
    except (OSError, ValueError):
        pass
    if not available and hasattr(os, "sysconf"):
        available = os.sysconf("SC_PAGE_SIZE") * os.sysconf("SC_AVPHYS_PAGES")
    logical_cpus = os.cpu_count() or 1
    memory_jobs = available // (1536 * 1024 * 1024) if available else 1
    return Parallelism(
        logical_cpus,
        max(1, min(logical_cpus, memory_jobs)),
        max(1, memory_jobs),
        available,
    )


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
    jobs: int = field(default_factory=lambda: detect_parallelism().jobs)


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

    expected_seconds estimates one build() invocation for each build type;
    expected_jobs records the parallelism used to obtain those estimates.
    The report generator uses both fields to allocate a global time budget.

    performance_build optionally provides a smaller build for performance
    diffs, with performance_phases naming its reported phases. Regular
    benchmarks and compile checks continue to use build().
    """

    version: str
    build: Callable[[Toolchain], dict[str, float]]
    expected_seconds: dict[str, float]
    expected_jobs: int = 20
    phases: tuple[str, ...] = PHASES
    comment: str = ""
    comments: dict[str, str] = field(default_factory=dict)
    performance_build: Callable[[Toolchain], dict[str, float]] | None = None
    performance_phases: tuple[str, ...] | None = None


def _run(cmd: list[str], cwd: Path, env: dict[str, str]) -> None:
    r = subprocess.run(
        cmd,
        cwd=cwd,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    if r.returncode != 0:
        print(f"failed command {cmd[0]}!\noutput:")
        print(r.stdout)
        r.check_returncode()


def _timed(cmd: list[str], cwd: Path, env: dict[str, str]) -> float:
    t0 = time.monotonic()
    _run(cmd, cwd, env)
    return (time.monotonic() - t0) * 1000.0


def _timed_many(
    cmds: list[list[str]], cwd: Path, env: dict[str, str], jobs: int
) -> float:
    """Wall-clock time to run many independent commands (e.g. compiling one
    file each) concurrently across CPUs -- unlike summing each command's own
    elapsed time, this reflects the phase's actual wall-clock duration."""
    t0 = time.monotonic()
    with ThreadPoolExecutor(max_workers=jobs) as pool:
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
            jobs = f"-j{tc.jobs}"
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
        expected_seconds={"debug": 10, "release": 13},
        phases=("compile", "run tests"),
        comments={"run tests": "the approval tests are ignored"},
    )


# --- abseil ------------------------------------------------------------


def _abseil(full: bool) -> Project:
    version = "20260107.1"
    url = f"https://github.com/abseil/abseil-cpp/archive/refs/tags/{version}.tar.gz"
    checksum = "4314e2a7cbac89cac25a2f2322870f343d81579756ceff7f431803c2c9090195"
    googletest_version = "1.17.0"
    googletest_url = (
        "https://github.com/google/googletest/releases/download/"
        f"v{googletest_version}/googletest-{googletest_version}.tar.gz"
    )
    googletest_checksum = (
        "65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c"
    )

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"abseil-cpp-{version}.tar.gz"
        _fetch(url, tarball, checksum)
        googletest_tarball = RW_DIR / f"googletest-{googletest_version}.tar.gz"
        _fetch(googletest_url, googletest_tarball, googletest_checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-abseil-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"abseil-cpp-{version}"
            with tarfile.open(googletest_tarball) as t:
                t.extractall(work)
            googletest_src = work / f"googletest-{googletest_version}"

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
                "-DABSL_LOCAL_GOOGLETEST_DIR=" + str(googletest_src),
                "-DBUILD_SHARED_LIBS=OFF",
            ]
            jobs = f"-j{tc.jobs}"
            base_library_targets = [
                "base",
                "raw_logging_internal",
                "spinlock_wait",
                "malloc_internal",
                "throw_delegate",
                "scoped_set_env",
                "strerror",
                "tracing_internal",
            ]
            base_test_targets = [
                "absl_atomic_hook_test",
                "absl_attributes_test",
                "absl_bit_cast_test",
                "absl_casts_test",
                "absl_errno_saver_test",
                "absl_throw_delegate_test",
                "absl_endian_test",
                "absl_no_destructor_test",
            ]
            additional_test_targets = (
                [
                    "absl_nullability_test",
                    "absl_nullability_default_nonnull_test",
                    "absl_nullability_traits_test",
                    "absl_scoped_set_env_test",
                    "absl_cmake_thread_test",
                    "absl_fast_type_id_test",
                    "absl_prefetch_test",
                    "absl_optimization_test",
                    "absl_poison_test",
                    "absl_tracing_internal_weak_test",
                    "absl_tracing_internal_strong_test",
                    "absl_iterator_traits_test",
                    "absl_algorithm_test",
                    "absl_cleanup_test",
                    "absl_compressed_tuple_test",
                    "absl_test_instance_tracker_test",
                    "absl_hashtable_control_bytes_test",
                    "absl_raw_hash_set_resize_impl_test",
                    "absl_non_temporal_memcpy_test",
                    "absl_bounded_utf8_length_sequence_test",
                    "absl_decode_rust_punycode_test",
                    "absl_demangle_rust_test",
                    "absl_utf8_for_code_point_test",
                    "absl_city_test",
                    "absl_memory_test",
                    "absl_constexpr_testing_test",
                    "absl_periodic_sampler_test",
                    "absl_random_internal_traits_test",
                    "absl_random_internal_fastmath_test",
                    "absl_random_internal_fast_uniform_bits_test",
                    "absl_random_internal_randen_test",
                    "absl_random_internal_randen_slow_test",
                    "absl_random_internal_uniform_helper_test",
                    "absl_random_internal_wide_multiply_test",
                    "absl_has_ostream_operator_test",
                    "absl_utf8_test",
                    "absl_ostringstream_test",
                    "absl_resize_uninitialized_test",
                    "absl_compare_test",
                    "absl_container_memory_test",
                    "absl_absl_exception_safety_testing_test",
                    "absl_ascii_test",
                    "absl_barrier_test",
                    "absl_bind_front_test",
                    "absl_bits_test",
                    "absl_blocking_counter_test",
                    "absl_borrowed_fixup_buffer_test",
                    "absl_call_once_test",
                    "absl_charconv_bigint_test",
                    "absl_charset_test",
                    "absl_chunked_queue_test",
                    "absl_config_test",
                    "absl_container_test",
                    "absl_cordz_update_tracker_test",
                    "absl_damerau_levenshtein_distance_test",
                    "absl_exponential_biased_test",
                    "absl_fixed_array_exception_safety_test",
                    "absl_flags_config_test",
                    "absl_flags_path_util_test",
                    "absl_flags_program_name_test",
                    "absl_flags_usage_config_test",
                    "absl_has_absl_stringify_test",
                    "absl_hash_policy_testing_test",
                    "absl_hashtablez_sampler_test",
                    "absl_inlined_vector_exception_safety_test",
                    "absl_internal_fnmatch_test",
                    "absl_kernel_timeout_internal_test",
                    "absl_log_internal_structured_proto_test",
                    "absl_low_level_hash_test",
                    "absl_match_test",
                    "absl_memutil_test",
                    "absl_mutex_method_pointer_test",
                    "absl_node_slot_policy_test",
                    "absl_notification_test",
                    "absl_overload_test",
                    "absl_per_thread_sem_test",
                    "absl_random_bernoulli_distribution_test",
                    "absl_random_bit_gen_ref_test",
                    "absl_random_generators_test",
                    "absl_random_internal_pcg_engine_test",
                    "absl_random_internal_seed_material_test",
                    "absl_raw_logging_test",
                    "absl_sample_recorder_test",
                    "absl_spinlock_test",
                    "absl_stacktrace_test",
                    "absl_str_replace_test",
                    "absl_strerror_test",
                    "absl_string_constant_test",
                    "absl_string_view_test",
                    "absl_strip_test",
                    "absl_substitute_test",
                    "absl_sysinfo_test",
                    "absl_thread_identity_test",
                    "absl_type_traits_test",
                    "absl_absl_check_test",
                    "absl_absl_log_basic_test",
                    "absl_char_formatting_test",
                    "absl_charconv_parse_test",
                    "absl_charconv_test",
                    "absl_check_test",
                    "absl_cord_data_edge_test",
                    "absl_cord_rep_btree_navigator_test",
                    "absl_cord_rep_btree_reader_test",
                    "absl_cord_rep_btree_test",
                    "absl_cord_rep_crc_test",
                    "absl_cordz_functions_test",
                    "absl_cordz_info_statistics_test",
                    "absl_cordz_info_test",
                    "absl_cordz_sample_token_test",
                    "absl_cordz_test",
                    "absl_cordz_update_scope_test",
                    "absl_crc32c_test",
                    "absl_crc_cord_state_test",
                    "absl_crc_memcpy_test",
                    "absl_demangle_test",
                    "absl_die_if_null_test",
                    "absl_escaping_test",
                    "absl_failure_signal_handler_test",
                    "absl_fixed_array_test",
                    "absl_flag_test",
                    "absl_flags_commandlineflag_test",
                    "absl_flags_flag_test",
                    "absl_flags_marshalling_test",
                    "absl_flags_parse_test",
                    "absl_flags_reflection_test",
                    "absl_flags_sequence_lock_test",
                    "absl_flags_usage_test",
                    "absl_function_ref_test",
                    "absl_hash_function_defaults_test",
                    "absl_layout_test",
                    "absl_leak_check_test",
                    "absl_lifetime_test",
                    "absl_log_basic_test",
                    "absl_log_entry_test",
                    "absl_log_flags_test",
                    "absl_log_globals_test",
                    "absl_log_internal_stderr_log_sink_test",
                    "absl_log_macro_hygiene_test",
                    "absl_log_modifier_methods_test",
                    "absl_log_severity_test",
                    "absl_log_sink_test",
                    "absl_log_streamer_test",
                    "absl_log_stripping_test",
                    "absl_low_level_alloc_test",
                    "absl_numbers_test",
                    "absl_pow10_helper_test",
                    "absl_random_discrete_distribution_test",
                    "absl_random_distributions_test",
                    "absl_random_exponential_distribution_test",
                    "absl_random_gaussian_distribution_test",
                    "absl_random_internal_chi_square_test",
                    "absl_random_internal_distribution_test_util_test",
                    "absl_random_internal_entropy_pool_test",
                    "absl_random_internal_generate_real_test",
                    "absl_random_internal_iostream_state_saver_test",
                    "absl_random_internal_randen_engine_test",
                    "absl_random_internal_randen_hwaes_test",
                    "absl_random_log_uniform_int_distribution_test",
                    "absl_random_mock_distributions_test",
                    "absl_random_mocking_bit_gen_test",
                    "absl_random_uniform_int_distribution_test",
                    "absl_random_zipf_distribution_test",
                    "absl_raw_hash_set_allocator_test",
                    "absl_raw_hash_set_test",
                    "absl_requires_test",
                    "absl_sample_element_size_test",
                    "absl_scoped_mock_log_test",
                    "absl_span_test",
                    "absl_span_test_noexceptions",
                    "absl_stack_consumption_test",
                    "absl_status_matchers_test",
                    "absl_status_matchers_with_unqualified_macros_test",
                    "absl_status_test",
                    "absl_str_cat_test",
                    "absl_str_format_arg_test",
                    "absl_str_format_bind_test",
                    "absl_str_format_checker_test",
                    "absl_str_format_extension_test",
                    "absl_str_format_output_test",
                    "absl_str_format_parser_test",
                    "absl_str_format_test",
                    "absl_strings_append_and_overwrite_test",
                    "absl_strings_resize_and_overwrite_test",
                    "absl_symbolize_test",
                    "absl_vlog_is_on_test",
                ]
                if full
                else []
            )
            test_targets = [*base_test_targets, *additional_test_targets]
            additional_library_targets = (
                [
                    "atomic_hook_test_helper",
                    "city",
                    "civil_time",
                    "cordz_functions",
                    "crc_cpu_detect",
                    "crc_internal",
                    "debugging_internal",
                    "decode_rust_punycode",
                    "demangle_internal",
                    "demangle_rust",
                    "exponential_biased",
                    "flags_commandlineflag_internal",
                    "graphcycles_internal",
                    "int128",
                    "leak_check",
                    "log_internal_conditions",
                    "log_internal_nullguard",
                    "log_severity",
                    "periodic_sampler",
                    "poison",
                    "pow10_helper",
                    "random_internal_platform",
                    "random_internal_randen",
                    "random_internal_randen_hwaes",
                    "random_internal_randen_hwaes_impl",
                    "random_internal_randen_slow",
                    "random_seed_gen_exception",
                    "stack_consumption",
                    "strings_internal",
                    "test_instance_tracker",
                    "time_zone",
                    "utf8_for_code_point",
                    "borrowed_fixup_buffer",
                    "cordz_handle",
                    "examine_stack",
                    "exception_safety_testing",
                    "failure_signal_handler",
                    "flags_commandlineflag",
                    "flags_config",
                    "flags_private_handle_accessor",
                    "flags_program_name",
                    "hash",
                    "hash_generator_testing",
                    "hashtablez_sampler",
                    "kernel_timeout_internal",
                    "log_entry",
                    "log_globals",
                    "log_initialize",
                    "log_internal_fnmatch",
                    "log_internal_globals",
                    "log_internal_log_sink_set",
                    "log_internal_proto",
                    "log_internal_structured_proto",
                    "log_internal_test_actions",
                    "log_internal_test_helpers",
                    "log_internal_test_matchers",
                    "log_sink",
                    "per_thread_sem_test_common",
                    "random_distributions",
                    "random_internal_entropy_pool",
                    "random_internal_seed_material",
                    "random_seed_sequences",
                    "scoped_mock_log",
                    "spinlock_test_common",
                    "stacktrace",
                    "strings",
                    "symbolize",
                    "synchronization",
                    "time",
                    "time_internal_test_util",
                    "vlog_config_internal",
                    "cord",
                    "cord_internal",
                    "cordz_info",
                    "cordz_sample_token",
                    "crc32c",
                    "crc_cord_state",
                    "die_if_null",
                    "flags_internal",
                    "flags_marshalling",
                    "flags_parse",
                    "flags_reflection",
                    "flags_usage",
                    "flags_usage_internal",
                    "generic_printer_internal",
                    "hashtable_profiler",
                    "log_flags",
                    "log_internal_check_op",
                    "log_internal_format",
                    "log_internal_message",
                    "profile_builder",
                    "random_internal_distribution_test_util",
                    "raw_hash_set",
                    "status",
                    "status_matchers",
                    "statusor",
                    "str_format_internal",
                ]
                if full
                else []
            )
            compile_targets = [
                *base_library_targets,
                *additional_library_targets,
                *test_targets,
            ]
            compile_command = (
                ["cmake", "--build", "build", jobs]
                if full
                else ["ninja", "-C", "build", jobs, *compile_targets]
            )
            test_filter = [] if full else ["-R", "^(" + "|".join(test_targets) + ")$"]

            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(compile_command, src, env),
                "run tests": _timed(
                    [
                        "ctest",
                        "--test-dir",
                        "build",
                        "--output-on-failure",
                        *test_filter,
                        jobs,
                    ],
                    src,
                    env,
                ),
            }

    return Project(
        version=f"{version} (full)" if full else version,
        build=build,
        expected_seconds=(
            {"debug": 25, "release": 35} if full else {"debug": 14, "release": 19}
        ),
        comment=(
            "Builds every default Abseil target and runs its complete "
            "configured upstream test suite."
            if full
            else "Builds absl/base and runs eight small upstream base tests."
        ),
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
            jobs = f"-j{tc.jobs}"
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
        expected_seconds={"debug": 32, "release": 69},
        comment="Builds GoogleTest's upstream unit tests with GMock and samples "
        "disabled, then runs the resulting CTest suite.",
    )


# --- godot -------------------------------------------------------------


def _godot() -> Project:
    version = "4.6.3-stable"
    url = f"https://github.com/godotengine/godot/archive/refs/tags/{version}.tar.gz"
    checksum = "fa22b5f974125057087c9ef725eae582dbc5e39385dc377e8d5dbc295b367e1c"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"godot-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-godot-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"godot-{version}"
            wrapper_name = (
                "clang++"
                if any("clang++" in Path(arg).name for arg in shlex.split(tc.cxx))
                else "cxx"
            )
            wrapper = _compiler_wrapper(work / wrapper_name, tc)
            target = (
                "template_debug" if tc.build_type == "debug" else "template_release"
            )
            suffix = f"linuxbsd.{target}.x86_64"
            if wrapper_name == "clang++":
                suffix += ".llvm"
            env = _env(tc)
            command = [
                "scons",
                f"-j{tc.jobs}",
                "platform=linuxbsd",
                "arch=x86_64",
                "target=" + target,
                "debug_symbols=no",
                "modules_enabled_by_default=no",
                "disable_3d=yes",
                "vulkan=no",
                "opengl3=no",
                "x11=no",
                "wayland=no",
                "sdl=no",
                "accesskit=no",
                "alsa=no",
                "pulseaudio=no",
                "dbus=no",
                "speechd=no",
                "fontconfig=no",
                "udev=no",
                "use_static_cpp=no",
                "progress=no",
                "import_env_vars=CCACHE_DIR,CCACHE_TEMPDIR,CCACHE_DISABLE",
                "CXX=" + str(wrapper),
                f"bin/obj/core/libcore.{suffix}.a",
            ]
            return {"compile": _timed(command, src, env)}

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 120, "release": 120},
        phases=("compile",),
        comment="Builds Godot's core static library with SCons; rendering, "
        "window-system, audio, and optional engine modules are disabled.",
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
            jobs = f"-j{tc.jobs}"
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
                    (
                        r"^kwsys\.test(Configure|Status|SystemTools|"
                        r"CommandLineArguments1?|Directory|Encoding|"
                        r"SystemInformation)$"
                    ),
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
        expected_seconds={"debug": 65, "release": 70},
        comment="Builds upstream CMake's core static library together with "
        "its KWSys, std-compatibility, and JSON support targets, then runs "
        "the supported KWSys tests. OpenSSL and debugger support are disabled.",
    )


# --- cppcheck -----------------------------------------------------------


def _cppcheck() -> Project:
    version = "2.21.0"
    url = f"https://github.com/cppcheck-opensource/cppcheck/archive/refs/tags/{version}.tar.gz"
    checksum = "f028ff75ca5372738f3737c8b3e8611426a6526b6aea2ef01301ab0f5902f044"

    def build_impl(tc: Toolchain, run_tests: bool) -> dict[str, float]:
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
            cppflags = (
                "-Ilib -Ifrontend -Icli "
                "-isystem externals "
                "-isystem externals/picojson "
                "-isystem externals/simplecpp "
                "-isystem externals/tinyxml2 "
                "-DHAVE_EXECINFO_H=1"
            )
            jobs = f"-j{tc.jobs}"
            make_args = [
                jobs,
                "CXX=" + str(wrapper),
                "CPPFLAGS=" + cppflags,
                "CXXFLAGS=",
                "FILESDIR=/usr/local/share/Cppcheck",
                "LDFLAGS=" + tc.ldflags,
                "LIBS=" + tc.libs,
            ]
            timings = {"compile": _timed(["make", "all", *make_args], src, env)}
            if run_tests:
                # This test hard-codes libstdc++'s vector::at diagnostic;
                # psychicstd intentionally does not promise that wording.
                timings["run tests"] = _timed(
                    [
                        "./testrunner",
                        "-x",
                        "TestSymbolDatabase::getVariableFromVarIdBoundsCheck",
                    ],
                    src,
                    env,
                )
            return timings

    def build(tc: Toolchain) -> dict[str, float]:
        return build_impl(tc, run_tests=True)

    def performance_build(tc: Toolchain) -> dict[str, float]:
        return build_impl(tc, run_tests=False)

    return Project(
        version=version,
        build=build,
        performance_build=performance_build,
        performance_phases=("compile",),
        expected_seconds={"debug": 15, "release": 15},
        phases=("compile", "run tests"),
        comment="the complete native Makefile build is compiled and linked; "
        "Cppcheck's own test runner is run with one libstdc++ diagnostic "
        "wording test excluded.",
    )


# --- ctre ---------------------------------------------------------------


def _ctre() -> Project:
    version = "3.11.0"
    url = (
        "https://github.com/hanickadot/compile-time-regular-expressions/"
        f"archive/refs/tags/v{version}.tar.gz"
    )
    checksum = "7d4b30d0bdd8864a47cceb2ab8e7c4d1846f0ec62383f8c45122435d32f19530"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"ctre-v{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-ctre-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"compile-time-regular-expressions-{version}"

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
                "-DCTRE_BUILD_TESTS=ON",
                "-DCTRE_BUILD_PACKAGE=OFF",
            ]
            jobs = f"-j{tc.jobs}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(
                    ["cmake", "--build", "build", "--target", "ctre-test", jobs],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 5, "release": 5},
        phases=("compile",),
        comment="Builds every upstream CTRE compile-time test.",
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
            # Keep randomized tests reproducible across benchmark variants.
            env["EIGEN_SEED"] = "1"
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
        expected_seconds={"debug": 15, "release": 15},
        phases=("compile", "run tests"),
        comment="eigen has no configure step; a fixed subset of its test "
        "suite is compiled and run individually, with times summed.",
    )


# --- flatbuffers ------------------------------------------------------


def _flatbuffers() -> Project:
    version = "25.12.19"
    url = f"https://github.com/google/flatbuffers/archive/refs/tags/v{version}.tar.gz"
    checksum = "f81c3162b1046fe8b84b9a0dbdd383e24fdbcf88583b9cb6028f90d04d90696a"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"flatbuffers-v{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-flatbuffers-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"flatbuffers-{version}"

            # sample_binary.cpp relies on a transitive declaration of printf.
            sample = src / "samples" / "sample_binary.cpp"
            sample.write_text("#include <cstdio>\n" + sample.read_text())

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
                "-DFLATBUFFERS_CPP_STD=20",
                "-DFLATBUFFERS_BUILD_TESTS=ON",
                "-DFLATBUFFERS_BUILD_GRPCTEST=OFF",
                "-DFLATBUFFERS_BUILD_BENCHMARKS=OFF",
                "-DFLATBUFFERS_BUILD_SHAREDLIB=OFF",
                "-DFLATBUFFERS_INSTALL=OFF",
            ]
            jobs = f"-j{tc.jobs}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(["cmake", "--build", "build", jobs], src, env),
                "run tests": _timed(
                    [
                        "ctest",
                        "--test-dir",
                        "build",
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
        expected_seconds={"debug": 30, "release": 30},
        comment="Builds the FlatBuffers compiler, library, samples, and upstream "
        "C++ test suite.",
    )


# --- fmt --------------------------------------------------------------


def _fmt() -> Project:
    version = "12.1.0"
    url = f"https://github.com/fmtlib/fmt/archive/refs/tags/{version}.tar.gz"
    checksum = "ea7de4299689e12b6dddd392f9896f08fb0777ac7168897a244a6d6085043fea"

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
            jobs = f"-j{tc.jobs}"
            configure_ms = _timed(configure, src, env)
            compile_ms = _timed(["cmake", "--build", "build", jobs], src, env)

            run_tests_ms = _timed(
                [
                    "ctest",
                    "--test-dir",
                    "build",
                    "--output-on-failure",
                    "-j",
                    str(tc.jobs),
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
        expected_seconds={"debug": 12, "release": 25},
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
            jobs = f"-j{tc.jobs}"
            configure_ms = _timed(configure, src, env)
            compile_ms = _timed(["cmake", "--build", "build", jobs], src, env)

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
            examples_ms = _timed_many(example_cmds, src, env, tc.jobs)
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

            return {
                "configure": configure_ms,
                "compile": compile_ms,
                "run tests": run_tests_ms,
                "examples": examples_ms,
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 57, "release": 60},
        phases=("configure", "compile", "run tests", "examples"),
        comments={
            "run tests": "unicode/cbor/msgpack (slow), algorithms (unspecified "
            "tail order), cmake_fetch/cmake_import (not applicable) excluded",
            "examples": "217 documented API examples, compiled but not run",
        },
    )


# --- small header-only projects ---------------------------------------


def _single_source_project(
    name: str,
    version: str,
    url: str,
    checksum: str,
    source_dir: str,
    source_files: tuple[str, ...],
    include_dirs: tuple[str, ...],
    run_args: tuple[str, ...] = (),
    extra_cxxflags: tuple[str, ...] = (),
    run_binary: bool = True,
) -> Project:
    """Compile small upstream examples/tests individually.

    This keeps header-only projects representative without adding a second
    build-system recipe whose own machinery would dominate the measurement.
    """

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"{name}-{version}.tar.gz"
        _fetch(url, tarball, checksum)
        with tempfile.TemporaryDirectory(prefix=f"rw-{name}-") as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as archive:
                archive.extractall(work)
            src = work / source_dir
            env = _env(tc)
            includes = ["-I" + str(src / d) for d in include_dirs]
            compile_ms = 0.0
            run_ms = 0.0
            for index, relative in enumerate(source_files):
                binary = work / f"example-{index}"
                command = [
                    tc.cxx,
                    *tc.cxxflags.split(),
                    *extra_cxxflags,
                    *includes,
                    str(src / relative),
                ]
                command += tc.ldflags.split() + tc.libs.split() + ["-o", str(binary)]
                compile_ms += _timed(command, src, env)
                if run_binary:
                    run_ms += _timed([str(binary), *run_args], src, env)
            result = {"compile": compile_ms}
            if run_binary:
                result["run tests"] = run_ms
            return result

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 10, "release": 10},
        phases=("compile", "run tests") if run_binary else ("compile",),
    )


def _inipp() -> Project:
    return _single_source_project(
        "inipp",
        "1.0.13",
        "https://github.com/mcmtroffaes/inipp/archive/refs/tags/1.0.13.tar.gz",
        "656c5d82db48f4da8ed70482839ad0da95ea1d576a3c890c272e9b9e0fb89571",
        "inipp-1.0.13",
        ("unittest/headertest.cpp",),
        ("inipp",),
    )


def _cxxopts() -> Project:
    return _single_source_project(
        "cxxopts",
        "3.3.1",
        "https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.3.1.tar.gz",
        "3bfc70542c521d4b55a46429d808178916a579b28d048bd8c727ee76c39e2072",
        "cxxopts-3.3.1",
        ("src/example.cpp",),
        ("include",),
        ("--help",),
        ("-DCXXOPTS_NO_REGEX",),
        run_binary=False,
    )


def _pocketfft() -> Project:
    commit = "c90e55b3d529f8efa40ed01a20de22405f45fc65"
    return _single_source_project(
        "pocketfft",
        commit[:12],
        f"https://github.com/mreineck/pocketfft/archive/{commit}.tar.gz",
        "2d90016da31a8cc1dd0ac0c89c1f2fb55df8f4ccc3f0386dcfbad09ac6d1c3ba",
        f"pocketfft-{commit}",
        ("pocketfft_demo.cc",),
        (".",),
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
                " -Wno-error=sign-conversion -Wno-error=sign-compare"
            )
            if "clang" not in tc.cxx.lower():
                # Modern GCC emits false positives for RapidJSON's allocation
                # wrappers at -O3; upstream enables -Werror for its tests.
                cxxflags += (
                    " -Wno-error=maybe-uninitialized"
                    " -Wno-error=alloc-size-larger-than= -Wno-error=array-bounds"
                    " -Wno-error=stringop-overflow"
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
            jobs = f"-j{tc.jobs}"
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
        expected_seconds={"debug": 34, "release": 97},
        phases=("configure", "compile", "run example", "run tests"),
        comment="RapidJSON's examples, archivertest, and unit tests are built; "
        "simpledom and unittest are run.",
    )


# --- react native ------------------------------------------------------


def _react_native() -> Project:
    version = "0.83.8"
    url = (
        f"https://github.com/facebook/react-native/archive/refs/tags/v{version}.tar.gz"
    )
    checksum = "615329ab197c4ca25d571d2ebaae19edd58bea5a1a226bb5a6f714e31d2b4354"
    fast_float_version = "8.0.0"
    fast_float_url = (
        "https://github.com/fastfloat/fast_float/"
        f"archive/refs/tags/v{fast_float_version}.tar.gz"
    )
    fast_float_checksum = (
        "f312f2dc34c61e665f4b132c0307d6f70ad9420185fa831911bc24408acf625d"
    )
    googletest_version = "1.16.0"
    googletest_url = (
        "https://github.com/google/googletest/"
        f"archive/refs/tags/v{googletest_version}.tar.gz"
    )
    googletest_checksum = (
        "78c676fc63881529bf97bf9d45948d905a66833fbfa5318ea2cd7478cb98f399"
    )

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"react-native-v{version}.tar.gz"
        _fetch(url, tarball, checksum)
        fast_float_tarball = RW_DIR / f"fast_float-v{fast_float_version}.tar.gz"
        _fetch(fast_float_url, fast_float_tarball, fast_float_checksum)
        googletest_tarball = RW_DIR / f"googletest-{googletest_version}.tar.gz"
        _fetch(googletest_url, googletest_tarball, googletest_checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-react-native-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            for archive in (tarball, fast_float_tarball, googletest_tarball):
                with tarfile.open(archive) as t:
                    t.extractall(work)
            src = work / f"react-native-{version}"
            fast_float = work / f"fast_float-{fast_float_version}"
            googletest = work / f"googletest-{googletest_version}"
            driver = work / "driver"
            driver.mkdir()
            driver.joinpath("CMakeLists.txt").write_text(
                """cmake_minimum_required(VERSION 3.20)
project(react_native_host_core LANGUAGES CXX)

set(REACT_COMMON_DIR "${RN_SOURCE}/packages/react-native/ReactCommon")

add_library(fast_float INTERFACE)
target_include_directories(fast_float INTERFACE "${FAST_FLOAT_SOURCE}/include")
# The selected CSS and oscompat sources do not use these targets' symbols.
add_library(glog INTERFACE)
add_library(react_debug INTERFACE)
add_library(react_utils INTERFACE)

add_subdirectory("${REACT_COMMON_DIR}/yoga/yoga" yoga)
add_subdirectory("${REACT_COMMON_DIR}/oscompat" oscompat)
add_subdirectory("${REACT_COMMON_DIR}/react/renderer/css" css)
add_subdirectory("${GTEST_SOURCE}" googletest)

file(GLOB CSS_TESTS
  "${REACT_COMMON_DIR}/react/renderer/css/tests/*.cpp"
)
list(FILTER CSS_TESTS EXCLUDE REGEX
  "CSS(Color|Keyword|Length)Test.cpp$"
)
add_executable(react_renderer_css_tests ${CSS_TESTS})
target_link_libraries(
  react_renderer_css_tests
  PRIVATE gtest_main react_renderer_css
)

add_custom_target(
  react_native_core
  DEPENDS yogacore oscompat react_renderer_css_tests
)
"""
            )

            env = _env(tc)
            wrapper = _compiler_wrapper(work / "cxx", tc, ("-DGTEST_HAS_CXXABI_H_=1",))
            configure = [
                "cmake",
                "-S",
                str(driver),
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + str(wrapper),
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DRN_SOURCE=" + str(src),
                "-DFAST_FLOAT_SOURCE=" + str(fast_float),
                "-DGTEST_SOURCE=" + str(googletest),
            ]
            jobs = f"-j{tc.jobs}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(
                    [
                        "cmake",
                        "--build",
                        "build",
                        "--target",
                        "react_native_core",
                        jobs,
                    ],
                    src,
                    env,
                ),
                "run tests": _timed(
                    [str(src / "build" / "react_renderer_css_tests")],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 20, "release": 30},
        phases=("compile", "run tests"),
        comment="Builds Yoga, oscompat, and React renderer CSS tests from "
        "ReactCommon, then runs 302 CSS tests. Three files with 10 "
        "compile-time-only tests are excluded because psychicstd's compact "
        "variant does not support constexpr construction. The full platform "
        "build requires Android or Apple SDKs, generated code, Hermes, fbjni, "
        "and the remaining native dependencies.",
    )


# --- rdfind ---------------------------------------------------------------


_RDFIND_COMMIT = "cac59ade85de364074a5ef1898096117c21570cc"
# rdfind's autoconf build has no build-type concept -- an -O flag is its equivalent.
_RDFIND_OPT_FLAG = {"debug": "-O0", "release": "-O2"}


def _rdfind() -> Project:
    fromcommit = True

    if fromcommit:
        # from a certain commit hash
        psychicstrictlevel = -0
        commithash = _RDFIND_COMMIT
        url = f"https://github.com/pauldreik/rdfind/archive/{commithash}.tar.gz"
        checksum = "49d2b1458e91168aea5540d438ac91ca0d97f651d587632d21aca3c91fae1202"
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
            if tc.build_type == "release":
                configure.append("--disable-assert")
            if tc.ldflags:
                configure.append(f"LDFLAGS={tc.ldflags}")
            if tc.libs:
                configure.append(f"LIBS={tc.libs}")
            jobs = f"-j{tc.jobs}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(["make", jobs], src, env),
                "run tests": _timed(["make", "check"], src, env),
            }

    return Project(
        version=proj_version,
        build=build,
        expected_seconds={"debug": 6, "release": 4},
        comment="rdfind is an autoconf based project. It uses psychic strict mode.",
    )


# --- simdutf -----


def _simdutf(strict: bool, strict_label: str) -> Project:
    version = "master260724"  # compaitibility fixes have been merged, but are not yet in a release
    commithash = "f542909a257bb3a9947f8e9fcaaba68a3e1f3b8b"
    url = f"https://github.com/simdutf/simdutf/archive/{commithash}.tar.gz"
    checksum = "fc6053688d67dc6b90f4dfaea12c06d8fd8254fc56fe62b9ef00942ef19cdf33"
    compatibility_level = 0 if strict else 2
    label = f"{version} ({strict_label})"

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
                + f" -D_PSYCHICSTD_COMPATIBILITY_LEVEL={compatibility_level}",
                "-DCMAKE_EXE_LINKER_FLAGS=" + tc.ldflags,
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + libs,
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DBUILD_SHARED_LIBS=OFF",
            ]
            jobs = f"-j{tc.jobs}"
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
        version=label,
        build=build,
        expected_seconds={"debug": 27, "release": 20},
        phases=("compile", "run tests"),
        comment="simdutf code is mostly simd intrinsics.",
    )


# --- tesseract -----------------------------------------------------------


def _tesseract() -> Project:
    version = "5.5.2"
    url = (
        f"https://github.com/tesseract-ocr/tesseract/archive/refs/tags/{version}.tar.gz"
    )
    checksum = "6235ea0dae45ea137f59c09320406f5888383741924d98855bd2ce0d16b54f21"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"tesseract-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-tesseract-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"tesseract-{version}"

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
                "-DCMAKE_CXX_STANDARD=20",
                "-DCMAKE_CXX_FLAGS=" + tc.cxxflags,
                "-DCMAKE_EXE_LINKER_FLAGS=" + tc.ldflags,
                "-DCMAKE_CXX_STANDARD_LIBRARIES=" + tc.libs,
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DBUILD_TRAINING_TOOLS=OFF",
                "-DBUILD_TESTS=OFF",
                "-DGRAPHICS_DISABLED=ON",
                "-DDISABLED_LEGACY_ENGINE=OFF",
                "-DDISABLE_ARCHIVE=ON",
                "-DDISABLE_CURL=ON",
                "-DOPENMP_BUILD=OFF",
            ]
            jobs = f"-j{tc.jobs}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(
                    ["cmake", "--build", "build", "--target", "tesseract", jobs],
                    src,
                    env,
                ),
                "run tests": _timed(
                    [str(src / "build" / "bin" / "tesseract"), "--version"],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 60, "release": 60},
        phases=("compile", "run tests"),
        comment="Builds the Tesseract OCR library and command-line program, "
        "then runs its version check. Training tools, ScrollView graphics, "
        "libarchive, libcurl, and OpenMP are disabled; Leptonica is required.",
    )


# --- tensorflow ---------------------------------------------------------


_TENSORFLOW_PLATFORM_TARGETS = (
    "//tensorflow/core/platform:cpu_feature_guard",
    "//tensorflow/core/platform:env_time",
    "//tensorflow/core/platform:fingerprint",
    "//tensorflow/core/platform:platform_strings",
    "//tensorflow/core/platform:stringprintf",
)


def _tensorflow() -> Project:
    version = "2.21.0"
    url = (
        f"https://github.com/tensorflow/tensorflow/archive/refs/tags/v{version}.tar.gz"
    )
    checksum = "ef3568bb4865d6c1b2564fb5689c19b6b9a5311572cd1f2ff9198636a8520921"
    bazel_version = "7.7.0"
    bazel_url = (
        "https://github.com/bazelbuild/bazel/releases/download/"
        f"{bazel_version}/bazel-{bazel_version}-linux-x86_64"
    )
    bazel_checksum = "fe7e799cbc9140f986b063e06800a3d4c790525075c877d00a7112669824acbf"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"tensorflow-v{version}.tar.gz"
        _fetch(url, tarball, checksum)
        bazel = RW_DIR / f"bazel-{bazel_version}-linux-x86_64"
        _fetch(bazel_url, bazel, bazel_checksum)
        bazel.chmod(0o755)

        with tempfile.TemporaryDirectory(
            prefix="rw-tensorflow-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"tensorflow-{version}"
            output_base = work / "bazel-output"
            output_user_root = work / "bazel-user"
            repository_cache = RW_DIR / ".tensorflow-bazel-repository-cache"
            repository_cache.mkdir(exist_ok=True)

            flags = shlex.split(tc.cxxflags)
            cxxflags: list[str] = []
            psychicstd_include: Path | None = None
            i = 0
            while i < len(flags):
                if flags[i] == "-isystem" and i + 1 < len(flags):
                    psychicstd_include = Path(flags[i + 1])
                    i += 2
                elif flags[i].startswith("-isystem") and len(flags[i]) > 8:
                    psychicstd_include = Path(flags[i][8:])
                    i += 1
                else:
                    cxxflags.append(flags[i])
                    i += 1

            env = _env(tc)
            cxx = Path(shutil.which(shlex.split(tc.cxx)[0]) or tc.cxx)
            compiler_candidates = []
            if "clang++" in cxx.name:
                compiler_candidates.append(
                    cxx.with_name(cxx.name.replace("clang++", "clang"))
                )
            if "g++" in cxx.name:
                compiler_candidates.append(
                    cxx.with_name(cxx.name.replace("g++", "gcc"))
                )
            if cxx.name == "c++":
                compiler_candidates.append(cxx.with_name("cc"))
            compiler = str(
                next(
                    (
                        candidate
                        for candidate in compiler_candidates
                        if candidate.exists()
                    ),
                    cxx,
                )
            )
            bazel_prefix = [
                str(bazel),
                "--batch",
                "--output_user_root=" + str(output_user_root),
                "--output_base=" + str(output_base),
            ]
            bazel_options = [
                "--config=clang_local",
                "--jobs=" + str(tc.jobs),
                "--local_resources=memory=" + str(tc.jobs * 1536),
                "--repository_cache=" + str(repository_cache),
                "--action_env=CC=" + compiler,
                "--repo_env=CC=" + compiler,
                "--noshow_progress",
                *(["--config=dbg"] if tc.build_type == "debug" else []),
                *("--cxxopt=" + flag for flag in cxxflags),
            ]

            if psychicstd_include is not None:
                overlay = src / "psychicstd-overlay"
                shutil.copytree(psychicstd_include, overlay / "include")
                include_env = str(overlay / "include")
                bazel_options.extend(
                    [
                        "--action_env=CPLUS_INCLUDE_PATH=" + include_env,
                        "--repo_env=CPLUS_INCLUDE_PATH=" + include_env,
                    ]
                )

            configure_ms = _timed(
                [
                    *bazel_prefix,
                    "fetch",
                    *bazel_options,
                    *_TENSORFLOW_PLATFORM_TARGETS,
                ],
                src,
                env,
            )

            # Snappy uses std::less_equal without including <functional>.
            snappy = output_base / "external" / "snappy" / "snappy.cc"
            snappy.write_text("#include <functional>\n" + snappy.read_text())

            compile_ms = _timed(
                [
                    *bazel_prefix,
                    "build",
                    *bazel_options,
                    *_TENSORFLOW_PLATFORM_TARGETS,
                ],
                src,
                env,
            )
            return {"configure": configure_ms, "compile": compile_ms}

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 60, "release": 60},
        phases=("compile",),
        comment="Builds a focused set of TensorFlow core platform/base libraries "
        "with Bazel; Python, CUDA, kernels, and the full framework are excluded.",
    )


# --- electron ------------------------------------------------------------


_ELECTRON_OPT_FLAG = {"debug": "-O0", "release": "-O2"}


def _electron() -> Project:
    version = "43.2.0"
    url = f"https://github.com/electron/electron/archive/refs/tags/v{version}.tar.gz"
    checksum = "eba2128a73febacedf89fabc9dc7ceb9f12bfd4d5a4470acb8ddc247b0da90f5"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"electron-v{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-electron-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"electron-{version}"
            driver = work / "driver"
            for directory in (
                "base",
                "base/containers",
                "base/strings",
                "build",
                "sandbox/policy",
                "shell/common",
            ):
                driver.joinpath(directory).mkdir(parents=True, exist_ok=True)

            driver.joinpath("base/command_line.h").write_text(
                """#pragma once
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace base {
#define BUILDFLAG(flag) BUILDFLAG_INTERNAL_##flag()
#define BUILDFLAG_INTERNAL_IS_LINUX() 1
#define BUILDFLAG_INTERNAL_IS_WIN() 0
#define DCHECK(condition) ((void)sizeof(condition))
#define UNSAFE_BUFFERS(expression) (expression)

class CommandLine {
 public:
  using StringVector = std::vector<std::string>;
  using StringViewType = std::string_view;
  using CharType = char;

  CommandLine() = default;
  explicit CommandLine(StringVector args) : args_(std::move(args)) {}

  bool HasSwitch(std::string_view name) const {
    return switches_.contains(std::string{name});
  }
  void AppendSwitch(std::string_view name) {
    switches_.emplace(name);
  }
  const StringVector& argv() const { return args_; }

  static CommandLine* ForCurrentProcess() {
    static CommandLine current(StringVector{"electron", "from-current-process"});
    return &current;
  }

 private:
  StringVector args_;
  std::set<std::string> switches_;
};
}  // namespace base
"""
            )
            driver.joinpath("base/no_destructor.h").write_text(
                """#pragma once

namespace base {
template <typename T>
class NoDestructor {
 public:
  NoDestructor() = default;
  T& operator*() { return value_; }
  const T& operator*() const { return value_; }

 private:
  T value_;
};
}  // namespace base
"""
            )
            driver.joinpath("base/containers/to_vector.h").write_text("#pragma once\n")
            driver.joinpath("base/strings/utf_string_conversions.h").write_text(
                "#pragma once\n"
            )
            driver.joinpath("build/build_config.h").write_text("#pragma once\n")
            driver.joinpath("sandbox/policy/switches.h").write_text(
                """#pragma once
#include <string_view>

namespace sandbox::policy::switches {
inline constexpr std::string_view kNoSandbox = "no-sandbox";
}  // namespace sandbox::policy::switches
"""
            )
            driver.joinpath("shell/common/options_switches.h").write_text(
                """#pragma once
#include <string_view>

namespace electron::switches {
inline constexpr std::string_view kEnableSandbox = "enable-sandbox";
}  // namespace electron::switches
"""
            )
            driver.joinpath("main.cc").write_text(
                """#include "shell/app/command_line_args.h"
#include "shell/common/electron_command_line.h"

#include <cassert>
#include <string>
#include <vector>

int main() {
  using Args = std::vector<std::string>;
  assert(electron::CheckCommandLineArguments(Args{"electron", "app.js"}));
  assert(electron::CheckCommandLineArguments(Args{"electron", "app:test"}));
  assert(!electron::CheckCommandLineArguments(
      Args{"electron", "app:test", "--gpu-launcher=cmd"}));
  assert(electron::CheckCommandLineArguments(
      Args{"electron", "app:test", "--", "--gpu-launcher=cmd"}));
  assert(electron::CheckCommandLineArguments(
      Args{"electron", "c:", "--safe-after-drive-path"}));
  assert(electron::CheckCommandLineArguments(
      Args{"electron", "not a scheme:", "--safe"}));

  base::CommandLine defaults;
  assert(electron::IsSandboxEnabled(&defaults));
  base::CommandLine disabled;
  disabled.AppendSwitch("no-sandbox");
  assert(!electron::IsSandboxEnabled(&disabled));
  disabled.AppendSwitch("enable-sandbox");
  assert(electron::IsSandboxEnabled(&disabled));

  const char* original[] = {"electron", "app.js", "--inspect"};
  electron::ElectronCommandLine::Init(3, original);
  assert((electron::ElectronCommandLine::AsUtf8() ==
          Args{"electron", "app.js", "--inspect"}));
  electron::ElectronCommandLine::InitializeFromCommandLine();
  assert((electron::ElectronCommandLine::AsUtf8() ==
          Args{"electron", "from-current-process"}));
}
"""
            )

            env = _env(tc)
            binary = work / "electron-command-line-tests"
            compile = [
                tc.cxx,
                *shlex.split(tc.cxxflags),
                _ELECTRON_OPT_FLAG[tc.build_type],
                "-I",
                str(driver),
                "-I",
                str(src),
                str(src / "shell/app/command_line_args.cc"),
                str(src / "shell/common/electron_command_line.cc"),
                str(driver / "main.cc"),
                *shlex.split(tc.ldflags),
                *shlex.split(tc.libs),
                "-o",
                str(binary),
            ]
            return {
                "compile": _timed(compile, src, env),
                "run tests": _timed([str(binary)], src, env),
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 3, "release": 3},
        phases=("compile", "run tests"),
        comment="Builds Electron's core command-line validation and original "
        "argument storage unchanged, then runs focused checks for the "
        "protocol-handler argument guard, sandbox switches, and Linux command "
        "line initialization. Small Chromium interface shims keep this initial "
        "slice independent of the full Chromium checkout.",
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
        expected_seconds={"debug": 39, "release": 39},
        comments={
            "compile": "Representative upstream Asio tests are compiled and "
            "linked directly; unrelated Boost libraries are excluded.",
        },
    )


# --- boost test ----------------------------------------------------------


def _boost_test() -> Project:
    version = "1.91.0"
    url = f"https://archives.boost.io/release/{version}/source/boost_1_91_0.tar.gz"
    checksum = "5734305f40a76c30f951c9abd409a45a2a19fb546efe4162119250bbe4d3a463"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"boost_{version.replace('.', '_')}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-boost-test-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"boost_{version.replace('.', '_')}"

            env = _env(tc)
            test_dir = src / "libs" / "test" / "test"
            test_sources = [
                Path("smoke-ts/basic-smoke-test.cpp"),
                Path("smoke-ts/basic-smoke-test2.cpp"),
                Path("usage-variants-ts/single-header-test.cpp"),
            ]
            build_dir = work / "boost-test-build"
            build_dir.mkdir()
            common = [
                tc.cxx,
                *shlex.split(tc.cxxflags),
                "-I",
                str(src),
                "-DBOOST_ALL_NO_LIB=1",
                "-DBOOST_NO_AUTO_PTR=1",
                "-DBOOST_NO_CXX98_BINDERS=1",
                "-pthread",
            ]
            commands = [
                [
                    *common,
                    str(test_dir / name),
                    "-o",
                    str(build_dir / name.stem),
                    *shlex.split(tc.ldflags),
                    *shlex.split(tc.libs),
                ]
                for name in test_sources
            ]
            compile_ms = _timed_many(commands, test_dir, env, tc.jobs)
            t0 = time.monotonic()
            for name in test_sources:
                _run([str(build_dir / name.stem)], test_dir, env)
            run_ms = (time.monotonic() - t0) * 1000.0
            return {"compile": compile_ms, "run tests": run_ms}

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 30, "release": 30},
        phases=("compile", "run tests"),
        comment="Builds and runs Boost.Test's three upstream header-only smoke tests.",
    )


# --- bitcoin core --------------------------------------------------------


def _bitcoin() -> Project:
    version = "31.0"
    url = f"https://github.com/bitcoin/bitcoin/archive/refs/tags/v{version}.tar.gz"
    checksum = "884fd15f195df3d36ab9c7d8854be16c53d9e7596ec001c283626e0fc1837e67"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"bitcoin-v{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-bitcoin-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"bitcoin-{version}"
            crypto_cmake = src / "src" / "crypto" / "CMakeLists.txt"
            crypto_text = crypto_cmake.read_text()
            for source in ("muhash.cpp", "siphash.cpp"):
                crypto_text = crypto_text.replace(f"  {source}\n", "")
            crypto_cmake.write_text(crypto_text)
            driver = work / "driver"
            driver.mkdir()
            driver.joinpath("CMakeLists.txt").write_text(
                """cmake_minimum_required(VERSION 3.22)
project(bitcoin_crypto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(core_interface INTERFACE)
include_directories("${BITCOIN_SOURCE}/src")
add_subdirectory("${BITCOIN_SOURCE}/src/crypto" crypto)
"""
            )

            env = _env(tc)
            wrapper = _compiler_wrapper(work / "cxx", tc)
            configure = [
                "cmake",
                "-S",
                str(driver),
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + str(wrapper),
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                "-DBITCOIN_SOURCE=" + str(src),
            ]
            jobs = f"-j{tc.jobs}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(
                    [
                        "cmake",
                        "--build",
                        "build",
                        "--target",
                        "bitcoin_crypto",
                        jobs,
                    ],
                    src,
                    env,
                ),
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 10, "release": 10},
        phases=("compile",),
        comment="Builds Bitcoin Core's bitcoin_crypto primitives except MuHash "
        "and SipHash, whose include chains require charconv; node, wallet, GUI, "
        "networking, and external dependencies are excluded.",
    )


# --- llama.cpp -----------------------------------------------------------


def _llama_cpp() -> Project:
    version = "b9637"
    url = f"https://github.com/ggml-org/llama.cpp/archive/refs/tags/{version}.tar.gz"
    checksum = "762283319feb3de30886dc850d42f0e426b06600e7f9639d34e06506597309ca"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"llama.cpp-{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-llama-cpp-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"llama.cpp-{version}"

            # <cmath> guarantees these classifiers in std; their presence in
            # the global namespace is implementation-dependent.
            ops = src / "ggml" / "src" / "ggml-cpu" / "ops.cpp"
            ops_text = ops.read_text()
            ops.write_text(
                ops_text.replace("isnan(", "std::isnan(").replace(
                    "isinf(", "std::isinf("
                )
            )

            env = _env(tc)
            wrapper = _compiler_wrapper(work / "cxx", tc)
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
                "-DBUILD_SHARED_LIBS=OFF",
                "-DLLAMA_BUILD_COMMON=OFF",
                "-DLLAMA_BUILD_TESTS=OFF",
                "-DLLAMA_BUILD_TOOLS=OFF",
                "-DLLAMA_BUILD_EXAMPLES=OFF",
                "-DLLAMA_BUILD_SERVER=OFF",
                "-DGGML_CPU=ON",
                "-DGGML_OPENMP=OFF",
                "-DGGML_NATIVE=OFF",
                "-DGGML_CCACHE=OFF",
            ]
            jobs = f"-j{tc.jobs}"
            configure_ms = _timed(configure, src, env)
            compile_ms = _timed(
                [
                    "cmake",
                    "--build",
                    "build",
                    "--target",
                    "ggml-cpu",
                    jobs,
                ],
                src,
                env,
            )
            compile_ms += _timed(
                [
                    "cmake",
                    "--build",
                    "build",
                    "--target",
                    "src/CMakeFiles/llama.dir/llama-arch.cpp.o",
                    "src/CMakeFiles/llama.dir/llama-hparams.cpp.o",
                    jobs,
                ],
                src,
                env,
            )
            return {
                "configure": configure_ms,
                "compile": compile_ms,
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 10, "release": 10},
        phases=("compile",),
        comment="Builds llama.cpp's ggml-base and ggml-cpu libraries and compiles "
        "its model-architecture and hyperparameter implementations; accelerator "
        "backends, tools, examples, server, and tests are excluded.",
    )


# --- libcamera -----------------------------------------------------------


def _libcamera() -> Project:
    version = "0.7.2"
    url = (
        "https://gitlab.freedesktop.org/camera/libcamera/"
        f"-/archive/v{version}/libcamera-v{version}.tar.gz"
    )
    checksum = "64881a4bafbda02f34861153d8f0d42be9ff46f5598408f9b585b07a10c448c9"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"libcamera-v{version}.tar.gz"
        _fetch(url, tarball, checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-libcamera-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            src = work / f"libcamera-v{version}"

            # Upstream only recognizes libc++ and libstdc++. Its choice only
            # affects a disabled Python test, so use the libstdc++ path for
            # psychicstd too.
            meson_file = src / "meson.build"
            meson_text = meson_file.read_text()
            meson_text = meson_text.replace(
                "else\n    error('C++ standard library cannot be detected')\nendif",
                "else\n    cxx_stdlib = 'libstdc++'\nendif",
            )
            meson_file.write_text(meson_text)
            class_cpp = src / "src" / "libcamera" / "base" / "class.cpp"
            class_cpp.write_text("#include <utility>\n" + class_cpp.read_text())
            log_cpp = src / "src" / "libcamera" / "base" / "log.cpp"
            log_cpp.write_text("#include <cctype>\n" + log_cpp.read_text())
            clock_cpp = src / "src" / "libcamera" / "clock_recovery.cpp"
            clock_cpp.write_text("#include <cmath>\n" + clock_cpp.read_text())
            ipa_proxy = src / "src" / "libcamera" / "ipa_proxy.cpp"
            ipa_proxy.write_text("#include <cctype>\n" + ipa_proxy.read_text())

            env = _env(tc)
            wrapper = _compiler_wrapper(work / "cxx", tc)
            env["CXX"] = str(wrapper)
            configure = [
                "meson",
                "setup",
                "build",
                "--buildtype=" + tc.build_type,
                "--default-library=static",
                "-Dwerror=false",
                "-Dpipelines=uvcvideo",
                "-Dipas=",
                "-Dandroid=disabled",
                "-Dcam=disabled",
                "-Ddocumentation=disabled",
                "-Dgstreamer=disabled",
                "-Dlc-compliance=disabled",
                "-Dlibdw=disabled",
                "-Dlibunwind=disabled",
                "-Dpycamera=disabled",
                "-Dqcam=disabled",
                "-Dsoftisp-gpu=disabled",
                "-Dtest=false",
                "-Dtracing=disabled",
                "-Dudev=disabled",
                "-Dv4l2=disabled",
            ]
            jobs = f"-j{tc.jobs}"
            return {
                "configure": _timed(configure, src, env),
                "compile": _timed(["ninja", "-C", "build", jobs], src, env),
            }

    return Project(
        version=version,
        build=build,
        expected_seconds={"debug": 6, "release": 8},
        expected_jobs=14,
        phases=("compile",),
        comment="Builds libcamera's core libraries and UVC pipeline handler; "
        "hardware-dependent applications, optional integrations, bindings, "
        "and tests are disabled.",
    )


# --- opencv --------------------------------------------------------------


def _opencv() -> Project:
    version = "4.13.0"
    url = f"https://github.com/opencv/opencv/archive/refs/tags/{version}.tar.gz"
    checksum = "1d40ca017ea51c533cf9fd5cbde5b5fe7ae248291ddf2af99d4c17cf8e13017d"
    extra_url = (
        f"https://github.com/opencv/opencv_extra/archive/refs/tags/{version}.tar.gz"
    )
    extra_checksum = "73eda44b867b898c3266db6b0c31c1641a7b6ca6e46914c43508e780a7d56d66"

    def build(tc: Toolchain) -> dict[str, float]:
        tarball = RW_DIR / f"opencv-{version}.tar.gz"
        _fetch(url, tarball, checksum)
        extra_tarball = RW_DIR / f"opencv_extra-{version}.tar.gz"
        _fetch(extra_url, extra_tarball, extra_checksum)

        with tempfile.TemporaryDirectory(
            prefix="rw-opencv-", ignore_cleanup_errors=True
        ) as work_dir:
            work = Path(work_dir)
            with tarfile.open(tarball) as t:
                t.extractall(work)
            with tarfile.open(extra_tarball) as t:
                t.extractall(work)
            src = work / f"opencv-{version}"
            test_data = work / f"opencv_extra-{version}" / "testdata"

            env = _env(tc, OPENCV_TEST_DATA_PATH=str(test_data))
            # OpenCV enables a large collection of optional codecs, language
            # bindings, and hardware backends by default. They are unrelated
            # to the standard-library-heavy core and make this recipe depend
            # on whichever packages happen to be installed on the runner.
            # Its SIMD emulator tests also type-pun through incompatible
            # pointers, which GCC 13 misoptimizes at -O3 with strict aliasing.
            configure = [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-GNinja",
                "-DCMAKE_BUILD_TYPE=" + tc.build_type.capitalize(),
                "-DCMAKE_CXX_COMPILER=" + tc.cxx,
                "-DCMAKE_CXX_FLAGS=" + tc.cxxflags + " -fno-strict-aliasing",
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
                "-DWITH_OPENEXR=OFF",
                "-DBUILD_OPENEXR=OFF",
                # The imgproc unit tests use JPEG and PNG regression fixtures.
                "-DWITH_JPEG=ON",
                "-DWITH_PNG=ON",
                "-DWITH_TIFF=OFF",
                "-DWITH_WEBP=OFF",
                "-DWITH_OPENJPEG=OFF",
            ]
            jobs = f"-j{tc.jobs}"
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
        expected_seconds={"debug": 540, "release": 189},
        comment="Builds OpenCV's core and imgproc modules and runs their "
        "upstream tests; optional codecs, bindings, and hardware backends "
        "are disabled.",
    )


PROJECTS: dict[str, Project] = {
    "abseil": _abseil(full=False),
    "abseil-full": _abseil(full=True),
    "bitcoin": _bitcoin(),
    "boost-asio": _boost_asio(),
    "boost-test": _boost_test(),
    "catch2": _catch2(),
    "cmake": _cmake(),
    "cppcheck": _cppcheck(),
    "ctre": _ctre(),
    "eigen": _eigen(),
    "electron": _electron(),
    "flatbuffers": _flatbuffers(),
    "fmt": _fmt(),
    "godot": _godot(),
    "googletest": _googletest(),
    "inipp": _inipp(),
    "cxxopts": _cxxopts(),
    "libcamera": _libcamera(),
    "llama.cpp": _llama_cpp(),
    "nlohmann": _nlohmann(),
    "opencv": _opencv(),
    "pocketfft": _pocketfft(),
    "rapidjson": _rapidjson(),
    "react-native": _react_native(),
    "rdfind": _rdfind(),
    "simdutf-dropin": _simdutf(strict=False, strict_label="drop-in"),
    "simdutf": _simdutf(strict=True, strict_label="strict"),
    "tesseract": _tesseract(),
    "tensorflow": _tensorflow(),
}
