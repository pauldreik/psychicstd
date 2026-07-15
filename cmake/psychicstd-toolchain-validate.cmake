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
