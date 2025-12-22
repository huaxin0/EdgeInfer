#include "infer/infer_manager.h"
#include <thread>
#include <nlohmann/json.hpp>
#include <chrono>
#include <sstream>
#include<iomanip>
#include<iostream>
#include "utils/base64.h" 
#include "utils/image_processor.h"
using json = nlohmann::json;

std::string InferManager::infer(const std::string &input){
    auto start =std::chrono::high_resolution_clock::now(); //统计开始时间
    try{
        //解析输入json
        json req =json::parse(input);
        //选取字段,包含图像的字段，必须。
        if(!req.contains("image")){
            json erro;  //新的 JSON 对象,组织错误响应  等价于json error = json::object();
            erro["code"]=400; //给错误的json,增加400字段
            erro["msg"]="missing required field:image";
            return erro.dump();  //把 JSON 错误对象变成 HTTP Response Body
        }
    std::string image_base64 = req["image"];  //获取图像的属性
    //可以选取的字段,获取
    float conf_thresh = req.value("conf_thresh", 0.5f);  //置信度阈值
    float iou_thresh = req.value("iou_thresh", 0.45f);  //iou的阈值  
    std::string model_name = req.value("model", "yolov5s"); //模型的类型
    //调试debug
    std::cout << "[InferManager] ========== New Request ==========" << std::endl;
    std::cout << "[InferManager] Image size: " << image_base64.size() << " bytes" << std::endl;
    std::cout << "[InferManager] Model: " << model_name << std::endl;
    std::cout << "[InferManager] Conf thresh: " << conf_thresh << std::endl;
    std::cout << "[InferManager] IOU thresh: " << iou_thresh << std::endl;
    //============================后续待做=================================    
    // ========== 2. Base64解码 ==========
    // TODO: 后续实现
    std::cout << "[InferManager] Decoding base64..." << std::endl;
    std::vector<uint8_t> image_data = Base64::decode(image_base64);
    std::cout << "[InferManager] Decoded image data: " << image_data.size() << " bytes" << std::endl;    
    // ========== 3. 图像预处理 ==========
    // TODO: 后续实现
    std::cout << "[InferManager] Image preprocessing... (TODO)" << std::endl;
    std::vector<float> preprocessed = ImageProcessor::preprocess(image_data, 640, 640); 
    std::cout << "[InferManager] Preprocessed tensor size: " << preprocessed.size() << std::endl;  
    // ========== 4. 昇腾推理 ==========
    // TODO: 后续实现
    std::cout << "[InferManager] NPU inference... (TODO)" << std::endl;
        
    // ========== 5. 后处理 ==========
    // TODO: 后续实现
    std::cout << "[InferManager] Postprocessing... (TODO)" << std::endl;
        
    // ========== 6. 构造返回结果（模拟） ==========
    json response;
    response["code"] = 0;
    response["msg"] = "success";    
    // 模拟检测结果
    response["detections"] = json::array({
        {
            {"class_id", 0},
            {"class_name", "person"},
            {"confidence", 0.95},
            {"bbox", {100, 200, 300, 400}}
        },
        {
            {"class_id", 2},
            {"class_name", "car"},
            {"confidence", 0.88},
            {"bbox", {450, 300, 600, 500}}
        }
    });
    auto end = std::chrono::high_resolution_clock::now(); //方便统计处理时间
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);  //程序运行时间
    response["latency_ms"] = duration.count(); //时间写入回应
    response["image_size"] = {640, 640};  //输出形变的尺寸
    std::cout << "[InferManager] Inference done in " << duration.count() << " ms" << std::endl;
    std::cout << "[InferManager] ========================================" << std::endl;
    return response.dump();  // 转为JSON字符串
    }catch (const json::exception& e) {
        // JSON解析错误
        json error;
        error["code"] = 400;
        error["msg"] = std::string("JSON parse error: ") + e.what();
        std::cerr << "[InferManager] JSON Error: " << e.what() << std::endl;
        return error.dump();     
    }catch (const std::exception& e) {
        // 其他运行时错误
        json error;
        error["code"] = 500;
        error["msg"] = std::string("Inference error: ") + e.what();
        std::cerr << "[InferManager] Runtime Error: " << e.what() << std::endl;
        return error.dump();
    }

}