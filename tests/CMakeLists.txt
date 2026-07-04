# Each test file is compiled twice from the same source:
#   <name>_system     — normal build, uses the real libstdc++ headers
#   <name>_psychicstd — built with -nostdinc++ pointing at our include/, uses psychicstd headers
# Both must pass. That's how we verify our implementation behaves like the real one.

set(TESTS
    test_algorithm
    test_any
    test_array
    test_atomic
    test_chrono
    test_compare
    test_complex
    test_concepts
    test_cstddef
    test_cstdint
    test_deque
    test_exception
    test_forward_list
    test_fstream
    test_functional
    test_initializer_list
    test_iomanip
    test_ios
    test_iostream
    test_istream
    test_iterator
    test_limits
    test_list
    test_map
    test_memory
    test_mutex
    test_new
    test_numeric
    test_optional
    test_random
    test_ranges
    test_ratio
    test_regex
    test_set
    test_sstream
    test_stack
    test_stdexcept
    test_string
    test_string_view
    test_thread
    test_tuple
    test_type_traits
    test_typeinfo
    test_unordered_map
    test_unordered_set
    test_utility
    test_valarray
    test_vector
)

foreach(t ${TESTS})
    add_executable(${t}_system ${t}.cpp)
    target_compile_features(${t}_system PRIVATE cxx_std_23)
    add_test(NAME ${t}_system COMMAND ${t}_system)

    add_executable(${t}_psychicstd ${t}.cpp)
    target_link_libraries(${t}_psychicstd PRIVATE psychicstd::psychicstd)
    add_test(NAME ${t}_psychicstd COMMAND ${t}_psychicstd)
endforeach()

# Self-containment tests: every header must be usable on its own. Including
# just that one header (and nothing else) must compile. This catches headers
# that rely on another header having been included first. The source is
# generated per header, so new headers are picked up automatically. Runs at
# whatever compatibility level the build is configured with.
file(GLOB _psychic_headers CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/*)
foreach(_hdr ${_psychic_headers})
    get_filename_component(_h ${_hdr} NAME)
    # Skip internal implementation headers (reserved __ names) -- not meant to
    # be included directly.
    if(_h MATCHES "^__")
        continue()
    endif()
    set(_src ${CMAKE_CURRENT_BINARY_DIR}/self_contained/sc_${_h}.cpp)
    file(
        WRITE ${_src}
        "// Auto-generated. Verifies <${_h}> is self-contained.\n"
        "#include <${_h}>\n"
        "#include <${_h}> // second include verifies the include guard\n"
        "int main() {}\n"
    )
    add_executable(self_contained_${_h} ${_src})
    target_link_libraries(self_contained_${_h} PRIVATE psychicstd::psychicstd)
    add_test(NAME self_contained_${_h} COMMAND self_contained_${_h})
endforeach()

# Simulates a completely independent project consuming psychicstd via a bash
# script
add_test(
    NAME ext_project
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/external_project/run.sh ${CMAKE_SOURCE_DIR}
)
