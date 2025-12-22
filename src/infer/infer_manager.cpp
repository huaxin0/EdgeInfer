#include "infer/infer_manager.h"
#include <thread>
#include <chrono>


std::string InferManager::infer(const std::string &input){
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //模拟推理消耗的时间
    return "mock_result";
}