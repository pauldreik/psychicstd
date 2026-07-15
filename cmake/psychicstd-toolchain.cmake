# psychicstd CMake toolchain overlay.
#
# This file intentionally does not pick a compiler. The user, Conan, or a
# higher-level preset/toolchain should do that. Psychicstd only injects the
# standard-library replacement bits.
#
# Compose it by including this file after any generated toolchain such as
# Conan's conan_toolchain.cmake.

include_guard(GLOBAL)

if(NOT UNIX OR WIN32)
    message(FATAL_ERROR "psychicstd toolchain is supported on Linux and macOS only")
endif()

set(PSYCHICSTD_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/.."
    CACHE PATH
    "Path to the psychicstd repository root"
)
set(PSYCHICSTD_INCLUDE_DIR
    "${PSYCHICSTD_ROOT}/include"
    CACHE PATH
    "Path to the psychicstd include directory"
)

if(NOT EXISTS "${PSYCHICSTD_INCLUDE_DIR}")
    message(FATAL_ERROR "psychicstd include directory does not exist: ${PSYCHICSTD_INCLUDE_DIR}")
endif()

# Append to any existing user or Conan flags rather than replacing them. This
# keeps sanitizer flags and other project-specific options intact.
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -nostdinc++ -fvisibility=hidden -isystem ${PSYCHICSTD_INCLUDE_DIR}"
)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib++")
# C++ ABI runtime: libc++abi on macOS, libsupc++ (+ libatomic) elsewhere.
if(APPLE)
    set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lc++abi")
else()
    set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lsupc++ -latomic")
endif()

# Run a validation pass after the real compiler is identified.
set(_psychicstd_validate "${CMAKE_CURRENT_LIST_DIR}/psychicstd-toolchain-validate.cmake")
if(CMAKE_PROJECT_INCLUDE)
    list(APPEND CMAKE_PROJECT_INCLUDE "${_psychicstd_validate}")
else()
    set(CMAKE_PROJECT_INCLUDE "${_psychicstd_validate}")
endif()
