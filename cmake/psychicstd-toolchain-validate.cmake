include_guard(GLOBAL)

if(NOT UNIX OR WIN32)
    message(FATAL_ERROR "psychicstd toolchain is supported on Linux and macOS only")
endif()

if(NOT CMAKE_CXX_COMPILER_ID)
    return()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13")
        message(FATAL_ERROR "psychicstd toolchain requires GCC 13 or newer")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
    # Supported (Clang and AppleClang).
else()
    message(
        FATAL_ERROR
        "psychicstd toolchain supports only GCC 13+, Clang, and AppleClang; got ${CMAKE_CXX_COMPILER_ID}"
    )
endif()

# The overlay is used without changing the consuming project's CMake files.
# Supply psychicstd's compiled implementation to targets declared after
# project(), just as the toolchain already supplies its headers and ABI runtime.
get_property(_psychicstd_in_try_compile GLOBAL PROPERTY IN_TRY_COMPILE)
if(NOT _psychicstd_in_try_compile AND NOT TARGET _psychicstd_runtime)
    add_library(_psychicstd_runtime STATIC "${PSYCHICSTD_ROOT}/src/iostream.cpp")
    target_compile_features(_psychicstd_runtime PRIVATE cxx_std_20)
    link_libraries(_psychicstd_runtime)
endif()
