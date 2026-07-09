add_executable(bench_startup_system bench_startup.cpp)
target_compile_features(bench_startup_system PRIVATE cxx_std_23)

add_executable(bench_startup_psychicstd bench_startup.cpp)
target_link_libraries(bench_startup_psychicstd PRIVATE psychicstd::psychicstd)
target_compile_features(bench_startup_psychicstd PRIVATE cxx_std_23)

add_custom_target(
    startup_bench
    COMMAND
        python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_bench.py $<TARGET_FILE:bench_startup_system>
        $<TARGET_FILE:bench_startup_psychicstd>
    DEPENDS bench_startup_system bench_startup_psychicstd
    COMMENT "Running startup-time benchmark"
    VERBATIM
)
