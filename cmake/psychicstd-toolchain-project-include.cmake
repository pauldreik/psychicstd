foreach(_psychicstd_project_include IN LISTS _psychicstd_previous_project_includes)
    include("${_psychicstd_project_include}")
endforeach()

# Validate only after project() has identified the real compiler.
include("${CMAKE_CURRENT_LIST_DIR}/psychicstd-toolchain-validate.cmake")
