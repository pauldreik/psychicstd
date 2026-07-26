add_executable(bench_wordcounter ../../examples/wordcounter.cpp)
target_link_libraries(bench_wordcounter PRIVATE psychicstd::psychicstd)
psychicstd_enable_warnings(bench_wordcounter)

# fmt: off
add_custom_target(bench
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/benchmark_compile_time.py
            ${CMAKE_CXX_COMPILER}
    COMMENT "Running compile-time benchmarks"
    VERBATIM
)
# fmt: on
