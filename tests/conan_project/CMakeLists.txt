cmake_minimum_required(VERSION 3.20)
project(conan_psychicstd_example LANGUAGES CXX)

file(WRITE "${CMAKE_BINARY_DIR}/.gitignore" "*\n")

find_package(fmt REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE fmt::fmt)
