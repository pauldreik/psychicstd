cmake_minimum_required(VERSION 3.20)
project(my_app LANGUAGES CXX)
file(WRITE "${CMAKE_BINARY_DIR}/.gitignore" "*\n")
add_executable(my_app main.cpp)
