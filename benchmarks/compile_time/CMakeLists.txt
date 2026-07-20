add_executable(bench_wordcounter bench_wordcounter.cpp)
target_link_libraries(bench_wordcounter PRIVATE psychicstd::psychicstd)
psychicstd_enable_warnings(bench_wordcounter)

# fmt: off
add_custom_target(bench
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_bench.py
            ${CMAKE_CXX_COMPILER}
            ${CMAKE_SOURCE_DIR}/include
    COMMENT "Running compile-time benchmarks"
    VERBATIM
)
# fmt: on
