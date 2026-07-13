cmake_minimum_required(VERSION 3.20)
project(conan_psychicstd_example LANGUAGES CXX)

file(WRITE "${CMAKE_BINARY_DIR}/.gitignore" "*\n")

find_package(fmt REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE fmt::fmt)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(my_app PRIVATE -Wall -Wextra)
endif()
