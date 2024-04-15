cmake_minimum_required(VERSION 3.0.0)
project(example-array-managed-unique-ptr VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(../../include)

add_executable(main main.cpp)