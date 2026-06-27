# catch2_SOURCE_DIR is set by the root CMakeLists via FetchContent.
# Catch2 itself is compiled with the system STL.
# Test files are compiled with psychicstd — this validates that user test code
# compiles and links correctly when psychicstd replaces the system headers.

foreach(t test_basic test_algorithms test_strings)
    add_executable(validation_catch2_${t} ${t}.cpp)
    target_link_libraries(validation_catch2_${t} PRIVATE psychicstd::psychicstd Catch2::Catch2WithMain)
    add_test(NAME validation_catch2_${t} COMMAND validation_catch2_${t})
endforeach()
