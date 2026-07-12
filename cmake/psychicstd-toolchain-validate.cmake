include_guard(GLOBAL)

if(NOT UNIX OR APPLE OR WIN32)
    message(FATAL_ERROR "psychicstd toolchain is supported on Linux only")
endif()

if(NOT CMAKE_CXX_COMPILER_ID)
    return()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13")
        message(FATAL_ERROR "psychicstd toolchain requires GCC 13 or newer")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
    # Supported.
else()
    message(
        FATAL_ERROR
        "psychicstd toolchain supports only GCC 13+ and Clang on Linux; got ${CMAKE_CXX_COMPILER_ID}"
    )
endif()
