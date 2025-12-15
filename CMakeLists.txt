cmake_minimum_required(VERSION 3.10)
project(EdgeInfer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SRC_FILES
    src/*.cpp
)

add_executable(edgeinfer ${SRC_FILES})