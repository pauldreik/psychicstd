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
    add_library(
        _psychicstd_runtime
        STATIC
        "${PSYCHICSTD_ROOT}/src/cerr.cpp"
        "${PSYCHICSTD_ROOT}/src/cin.cpp"
        "${PSYCHICSTD_ROOT}/src/clog.cpp"
        "${PSYCHICSTD_ROOT}/src/cout.cpp"
        "${PSYCHICSTD_ROOT}/src/ios.cpp"
        "${PSYCHICSTD_ROOT}/src/iostream_macos.cpp"
        "${PSYCHICSTD_ROOT}/src/istream.cpp"
        "${PSYCHICSTD_ROOT}/src/ostream.cpp"
        "${PSYCHICSTD_ROOT}/src/sstream_instantiations.cpp"
        "${PSYCHICSTD_ROOT}/src/stdio_streambuf.cpp"
        "${PSYCHICSTD_ROOT}/src/stdexcept.cpp"
        "${PSYCHICSTD_ROOT}/src/string.cpp"
        "${PSYCHICSTD_ROOT}/src/string_instantiations.cpp"
        "${PSYCHICSTD_ROOT}/src/system_error.cpp"
    )
    set_target_properties(
        _psychicstd_runtime
        PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            POSITION_INDEPENDENT_CODE ON
    )
    foreach(_psychicstd_config IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${_psychicstd_config}" _psychicstd_config_upper)
        set_target_properties(
            _psychicstd_runtime
            PROPERTIES
                "ARCHIVE_OUTPUT_DIRECTORY_${_psychicstd_config_upper}" "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endforeach()
    target_compile_features(_psychicstd_runtime PRIVATE cxx_std_20)
    # Keep the private target out of package exports. The wrapper supplies only
    # build ordering; the archive goes after project libraries and before the
    # ABI libraries in CMAKE_CXX_STANDARD_LIBRARIES.
    add_library(_psychicstd_runtime_link INTERFACE IMPORTED GLOBAL)
    add_dependencies(_psychicstd_runtime_link _psychicstd_runtime)
    link_libraries("$<BUILD_INTERFACE:_psychicstd_runtime_link>")
    set(_psychicstd_runtime_archive
        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}_psychicstd_runtime${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )
    set(CMAKE_CXX_STANDARD_LIBRARIES
        "\"${_psychicstd_runtime_archive}\" ${CMAKE_CXX_STANDARD_LIBRARIES}"
    )
endif()
