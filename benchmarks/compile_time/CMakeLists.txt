# rapidjson_SOURCE_DIR is set by the root CMakeLists via FetchContent

# Development target: psychicstd + rapidjson so the IDE navigates to our headers.
# EXCLUDE_FROM_ALL: won't build until the missing std headers are implemented.
add_executable(bench_rapidjson bench_rapidjson.cpp)
set_target_properties(bench_rapidjson PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(bench_rapidjson PRIVATE psychicstd::psychicstd)
target_include_directories(bench_rapidjson PRIVATE ${rapidjson_SOURCE_DIR}/include)
psychicstd_enable_warnings(bench_rapidjson)

add_executable(bench_wordcounter bench_wordcounter.cpp)
target_link_libraries(bench_wordcounter PRIVATE psychicstd::psychicstd)
target_include_directories(bench_wordcounter PRIVATE ${rapidjson_SOURCE_DIR}/include)
psychicstd_enable_warnings(bench_wordcounter)

set(RJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson)

# fmt: off
add_custom_target(bench
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_bench.py
            ${CMAKE_CXX_COMPILER}
            ${CMAKE_SOURCE_DIR}/include
            --extra-include rapidjson:${rapidjson_SOURCE_DIR}/include
            --bench-file ${RJ_DIR}/test_parse.cpp:rj/parse:rapidjson
            --bench-file ${RJ_DIR}/test_write.cpp:rj/write:rapidjson
            --bench-file ${RJ_DIR}/test_dom.cpp:rj/dom:rapidjson
    COMMENT "Running compile-time benchmarks"
    VERBATIM
)
# fmt: on
