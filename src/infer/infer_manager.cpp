#include "infer/infer_manager.h"
#include "infer/ascend_infer.h"
#include "infer/post_processor.h"
#include "utils/base64.h"
#include "utils/image_processor.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>

using json = nlohmann::json;

InferManager::InferManager() 
    : model_path_("/root/Desktop/EdgeInfer/models/om/yolov8s.om") {
    
    std::cout << "[InferManager] Initializing..." << std::endl;
    
    // 创建昇腾推理引擎
    ascend_infer_ = std::make_shared<AscendInfer>(0);
    
    // 初始化ACL
    if (!ascend_infer_->init()) {
        std::cerr << "[InferManager] Failed to init AscendInfer" << std::endl;
        throw std::runtime_error("AscendInfer init failed");
    }
    
    // 加载模型
    if (!ascend_infer_->loadModel(model_path_)) {
        std::cerr << "[InferManager] Failed to load model: " << model_path_ << std::endl;
        throw std::runtime_error("Model load failed");
    }
    
    std::cout << "[InferManager] Initialization completed" << std::endl;
}

InferManager::~InferManager() {
    std::cout << "[InferManager] Destroying..." << std::endl;
}

std::string InferManager::infer(const std::string& input) {
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        // ========== 1. 解析输入JSON ==========
        json req = json::parse(input);
        
        if (!req.contains("image")) {
            json error;
            error["code"] = 400;
            error["msg"] = "missing required field: image";
            return error.dump();
        }
        
        std::string image_base64 = req["image"];
        float conf_thresh = req.value("conf_thresh", 0.5f);
        float iou_thresh = req.value("iou_thresh", 0.45f);
        std::string model_name = req.value("model", "yolov8s");
        
        std::cout << "\n[InferManager] ========== New Request ==========" << std::endl;
        std::cout << "[InferManager] Base64 size: " << image_base64.size() << " bytes" << std::endl;
        std::cout << "[InferManager] Model: " << model_name << std::endl;
        std::cout << "[InferManager] Conf thresh: " << conf_thresh << std::endl;
        std::cout << "[InferManager] IOU thresh: " << iou_thresh << std::endl;
        
        // ========== 2. Base64解码 ==========
        std::cout << "[InferManager] Decoding base64..." << std::endl;
        std::vector<uint8_t> image_data = Base64::decode(image_base64);
        std::cout << "[InferManager] Decoded image data: " << image_data.size() << " bytes" << std::endl;
        
        // ========== 3. 图像预处理 ==========
        std::cout << "[InferManager] Preprocessing image..." << std::endl;
        std::vector<float> preprocessed = ImageProcessor::preprocess(image_data, 640, 640);
        std::cout << "[InferManager] Preprocessed tensor size: " << preprocessed.size() << std::endl;
        
        // ========== 4. 昇腾推理 ==========
        std::cout << "[InferManager] Running NPU inference..." << std::endl;
        auto infer_start = std::chrono::high_resolution_clock::now();
        
        std::vector<float> raw_output = ascend_infer_->infer(preprocessed);
        
        auto infer_end = std::chrono::high_resolution_clock::now();
        auto infer_duration = std::chrono::duration_cast<std::chrono::milliseconds>(infer_end - infer_start);
        
        std::cout << "[InferManager] Raw output size: " << raw_output.size() << std::endl;
        std::cout << "[InferManager] NPU inference time: " << infer_duration.count() << " ms" << std::endl;
        
        // ========== 5. 后处理 ==========
        std::cout << "[InferManager] Post-processing..." << std::endl;
        std::vector<Detection> detections = PostProcessor::processYOLOv8(
            raw_output, conf_thresh, iou_thresh, 640, 640
        );
        
        // ========== 6. 构造返回结果 ==========
        json response;
        response["code"] = 0;
        response["msg"] = "success";
        
        json detections_json = json::array();
        for (const auto& det : detections) {
            json det_json;
            det_json["class_id"] = det.class_id;
            det_json["class_name"] = det.class_name;
            det_json["confidence"] = det.confidence;
            det_json["bbox"] = {det.x, det.y, det.w, det.h};
            detections_json.push_back(det_json);
        }
        response["detections"] = detections_json;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        response["latency_ms"] = duration.count();
        response["npu_latency_ms"] = infer_duration.count();
        response["image_size"] = {640, 640};
        
        std::cout << "[InferManager] Total latency: " << duration.count() << " ms" << std::endl;
        std::cout << "[InferManager] Detected " << detections.size() << " objects" << std::endl;
        std::cout << "[InferManager] ========================================\n" << std::endl;
        
        return response.dump();
        
    } catch (const json::exception& e) {
        json error;
        error["code"] = 400;
        error["msg"] = std::string("JSON parse error: ") + e.what();
        std::cerr << "[InferManager] JSON Error: " << e.what() << std::endl;
        return error.dump();
        
    } catch (const std::exception& e) {
        json error;
        error["code"] = 500;
        error["msg"] = std::string("Error: ") + e.what();
        std::cerr << "[InferManager] Error: " << e.what() << std::endl;
        return error.dump();
    }
}