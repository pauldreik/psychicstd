# Each test is compiled with psychicstd and run via ctest.
foreach(t test_parse test_write test_dom)
    add_executable(validation_rapidjson_${t} ${t}.cpp)
    target_link_libraries(validation_rapidjson_${t} PRIVATE psychicstd::psychicstd)
    target_include_directories(validation_rapidjson_${t} PRIVATE ${rapidjson_SOURCE_DIR}/include)
    add_test(NAME validation_rapidjson_${t} COMMAND validation_rapidjson_${t})
endforeach()
