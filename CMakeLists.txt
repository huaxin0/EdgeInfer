cmake_minimum_required(VERSION 3.10)
project(EdgeInfer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(edgeinfer
    src/main.cpp
    src/server/http_server.cpp
    src/infer/infer_manager.cpp
)

target_include_directories(edgeinfer
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/third_party
)
