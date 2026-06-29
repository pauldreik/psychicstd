# Each test file is compiled twice from the same source:
#   <name>_system     — normal build, uses the real libstdc++ headers
#   <name>_psychicstd — built with -nostdinc++ pointing at our include/, uses psychicstd headers
# Both must pass. That's how we verify our implementation behaves like the real one.

set(TESTS test_numeric test_cstdint test_cstddef test_memory)

foreach(t ${TESTS})
    add_executable(${t}_system ${t}.cpp)
    target_compile_features(${t}_system PRIVATE cxx_std_23)
    add_test(NAME ${t}_system COMMAND ${t}_system)

    add_executable(${t}_psychicstd ${t}.cpp)
    target_link_libraries(${t}_psychicstd PRIVATE psychicstd::psychicstd)
    add_test(NAME ${t}_psychicstd COMMAND ${t}_psychicstd)
endforeach()

# Simulates a completely independent project consuming psychicstd via a bash
# script
add_test(
    NAME ext_project
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/external_project/run.sh ${CMAKE_SOURCE_DIR}
)
