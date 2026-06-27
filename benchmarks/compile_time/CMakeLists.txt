# rapidjson_SOURCE_DIR is set by the root CMakeLists via FetchContent

# Development target: psychicstd + rapidjson so the IDE navigates to our headers.
# EXCLUDE_FROM_ALL: won't build until the missing std headers are implemented.
add_executable(bench_rapidjson bench_rapidjson.cpp)
set_target_properties(bench_rapidjson PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(bench_rapidjson PRIVATE psychicstd::psychicstd)
target_include_directories(bench_rapidjson PRIVATE ${rapidjson_SOURCE_DIR}/include)

set(RJ_DIR  ${CMAKE_SOURCE_DIR}/validation/rapidjson)
set(C2_DIR  ${CMAKE_SOURCE_DIR}/validation/catch2)

add_custom_target(bench
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_bench.py
            ${CMAKE_CXX_COMPILER}
            ${CMAKE_SOURCE_DIR}/include
            --extra-include rapidjson:${rapidjson_SOURCE_DIR}/include
            --extra-include catch2:${catch2_SOURCE_DIR}/src:${catch2_BINARY_DIR}/generated-includes
            --bench-file ${RJ_DIR}/test_parse.cpp:rj/parse:rapidjson
            --bench-file ${RJ_DIR}/test_write.cpp:rj/write:rapidjson
            --bench-file ${RJ_DIR}/test_dom.cpp:rj/dom:rapidjson
            --bench-file ${C2_DIR}/test_basic.cpp:catch2/basic:catch2
            --bench-file ${C2_DIR}/test_algorithms.cpp:catch2/algorithms:catch2
            --bench-file ${C2_DIR}/test_strings.cpp:catch2/strings:catch2
    COMMENT "Running compile-time benchmarks"
    VERBATIM
)
