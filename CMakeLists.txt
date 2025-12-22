cmake_minimum_required(VERSION 3.10)
project(EdgeInfer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -02")
#=================1. json库的载入==================
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# ========== 2. 昇腾路径配置 ==========
set(ASCEND_PATH /usr/local/Ascend/ascend-toolkit/latest)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
    ${ASCEND_PATH}/include  # 昇腾头文件
)

link_directories(
    ${ASCEND_PATH}/lib64  # 昇腾库文件
)

# ========== 3. 编译可执行文件 ==========
add_executable(edgeinfer
    src/main.cpp
    src/server/http_server.cpp
    src/infer/infer_manager.cpp
)

# ========== 4. 链接库 ==========
target_link_libraries(edgeinfer 
    PRIVATE 
        nlohmann_json::nlohmann_json  # JSON库
        ascendcl                       # 昇腾推理库
        acl_op_compiler                # 昇腾算子库
        pthread                        # 线程库
)
