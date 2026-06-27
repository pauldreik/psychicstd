cmake_minimum_required(VERSION 3.25)
project(psychicstd LANGUAGES CXX VERSION 0.1.0)

file(WRITE "${CMAKE_BINARY_DIR}/.gitignore" "*\n")

add_library(psychicstd INTERFACE)
add_library(psychicstd::psychicstd ALIAS psychicstd)

target_compile_features(psychicstd INTERFACE cxx_std_23)
target_compile_options(psychicstd INTERFACE -nostdinc++)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16")
    # GCC 12 lacks -nostdlib++ (added in later versions), so use -nodefaultlibs instead.
    # That strips everything, so add back: libsupc++ (C++ runtime ABI: operator
    # new/delete, __gxx_personality_v0), libm, libc, libgcc_s/libgcc.
    # --start-group/--end-group lets the linker resolve circular refs between
    # them.
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libsupc++.a
      OUTPUT_VARIABLE PSYCHICSTD_SUPCXX_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    target_link_options(psychicstd INTERFACE -nodefaultlibs)
    target_link_libraries(
      psychicstd
      INTERFACE -Wl,--start-group
                ${PSYCHICSTD_SUPCXX_PATH}
                -lm
                -lc
                -lgcc_s
                -lgcc
                -Wl,--end-group)
  else()
    message(
      FATAL_ERROR
        "GCC ${CMAKE_CXX_COMPILER_VERSION} is not yet supported; only GCC 12-15 are tested"
    )
  endif()
else()
  message(
    FATAL_ERROR
      "Only gcc is supported, ${CMAKE_CXX_COMPILER_ID} has not been tested yet")
endif()
target_include_directories(psychicstd INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
option(PSYCHICSTD_BUILD_TESTS      "Build tests"                   ON)
option(PSYCHICSTD_BUILD_BENCHMARKS "Build compile-time benchmarks" ON)
option(PSYCHICSTD_BUILD_VALIDATION "Build third-party validation"  ON)

# Third-party deps shared by benchmarks and validation
include(FetchContent)
FetchContent_Declare(rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
set(RAPIDJSON_BUILD_DOC      OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rapidjson)

FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.0
    GIT_SHALLOW    TRUE
)
set(CATCH_BUILD_TESTING  OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_DOCS   OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Catch2)
# Compile Catch2 with psychicstd so its std::string ABI matches our test files
target_link_libraries(Catch2 PRIVATE psychicstd::psychicstd)
target_link_libraries(Catch2WithMain PRIVATE psychicstd::psychicstd)

if(PSYCHICSTD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(PSYCHICSTD_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks/compile_time)
endif()
if(PSYCHICSTD_BUILD_VALIDATION)
    if(NOT PSYCHICSTD_BUILD_TESTS)
        enable_testing()
    endif()
    add_subdirectory(validation)
endif()
