cmake_minimum_required(VERSION 3.10)
project(EdgeInfer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

#=================1. json库==================
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

#=================2. OpenCV==================
find_package(OpenCV REQUIRED)

#=================3. 昇腾路径==================
set(ASCEND_PATH /usr/local/Ascend/ascend-toolkit/latest)

#=================4. include路径==================
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
    ${ASCEND_PATH}/include   # 昇腾头文件
    ${OpenCV_INCLUDE_DIRS}   # OpenCV头文件
)

link_directories(
    ${ASCEND_PATH}/lib64
)

#=================5. 可执行文件==================
add_executable(edgeinfer
    src/main.cpp
    src/server/http_server.cpp
    src/infer/infer_manager.cpp
    src/utils/base64.cpp
    src/utils/image_processor.cpp
    src/infer/ascend_infer.cpp
    src/infer/post_processor.cpp 
)

#=================6. 链接库==================
target_link_libraries(edgeinfer
    PRIVATE
        nlohmann_json::nlohmann_json
        ${OpenCV_LIBS}      # 注意这里改为 ${OpenCV_LIBS}，而不是 INCLUDE_DIRS
        ascendcl
        acl_op_compiler
        pthread
)
