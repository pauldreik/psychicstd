cmake_minimum_required(VERSION 3.25)
project(psychicstd LANGUAGES CXX VERSION 0.1.0)

file(WRITE "${CMAKE_BINARY_DIR}/.gitignore" "*\n")

add_library(
    psychicstd
    STATIC
    src/iostream.cpp
    src/stdexcept.cpp
    src/string.cpp
    src/string_instantiations.cpp
    src/system_error.cpp
)
add_library(psychicstd::psychicstd ALIAS psychicstd)
set_target_properties(psychicstd PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_features(psychicstd PUBLIC cxx_std_20)
target_compile_options(psychicstd PUBLIC -nostdinc++ -fvisibility=hidden)
if(APPLE)
    # atomic::wait uses os_sync_wait_on_address, introduced in macOS 14.4.
    target_compile_options(psychicstd PUBLIC -mmacosx-version-min=14.4)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    function(psychicstd_enable_warnings target)
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endfunction()
else()
    function(psychicstd_enable_warnings target)
    endfunction()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17")
        # GCC 12 lacks -nostdlib++ (added in later versions), so use -nodefaultlibs instead.
        # That strips everything, so add back: libsupc++ (C++ runtime ABI: operator
        # new/delete, __gxx_personality_v0), libm, libc, libgcc_s/libgcc.
        # GCC's own library search path is unaffected by -nodefaultlibs, so -lsupc++
        # is sufficient without a full path.
        target_link_options(psychicstd INTERFACE -nodefaultlibs)
        set(__psychicstd_libs
            -lsupc++
            -latomic
            -lm
            -lc
            -lgcc_s
            -lgcc
        )
        # Sanitizer runtimes must come before the gcc support libs so their
        # interceptors shadow the real functions.
        if(CMAKE_CXX_FLAGS MATCHES "-fsanitize=address")
            list(INSERT __psychicstd_libs 0 -lasan)
        endif()
        if(CMAKE_CXX_FLAGS MATCHES "-fsanitize=undefined")
            list(INSERT __psychicstd_libs 0 -lubsan)
        endif()
        target_link_libraries(psychicstd INTERFACE ${__psychicstd_libs})
    else()
        message(
            FATAL_ERROR
            "GCC ${CMAKE_CXX_COMPILER_VERSION} is not yet supported; only GCC 12-16 are tested"
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    # Clang and AppleClang support -nostdlib++ which strips libc++/libstdc++
    # from the default link set but keeps libc, libm, and the compiler runtime.
    target_link_options(psychicstd INTERFACE -nostdlib++)
    if(APPLE)
        # macOS ships the C++ ABI runtime as libc++abi (operator new/delete,
        # __gxx_personality_v0, RTTI, __cxa_*). No libsupc++/libatomic there.
        target_link_libraries(psychicstd INTERFACE -lc++abi)
    else()
        # Add libsupc++ back for the C++ ABI runtime (operator new/delete,
        # __gxx_personality_v0, RTTI).
        target_link_libraries(psychicstd INTERFACE -lsupc++ -latomic)
    endif()
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()
target_include_directories(
    psychicstd
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
option(PSYCHICSTD_BUILD_TESTS "Build tests" ON)
option(PSYCHICSTD_BUILD_BENCHMARKS "Build compile-time benchmarks" ON)

if(PSYCHICSTD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(PSYCHICSTD_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks/compile_time)
    add_subdirectory(benchmarks/startup_time)
endif()
