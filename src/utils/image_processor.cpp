#include "utils/image_processor.h"
#include <iostream>
#include <stdexcept>

std::vector<float> ImageProcessor::preprocess(
    const std::vector<uint8_t>& image_data,
    int target_h,
    int target_w
) {
    if (image_data.empty()) {
        throw std::runtime_error("Empty image data");
    }
    
    // 1. 解码图像（从内存中的JPEG/PNG字节流）
    cv::Mat image = cv::imdecode(image_data, cv::IMREAD_COLOR);
    
    if (image.empty()) {
        throw std::runtime_error("Failed to decode image");
    }
    
    std::cout << "[ImageProcessor] Decoded image: " 
              << image.cols << "x" << image.rows 
              << " channels=" << image.channels() << std::endl;
    
    return process_image(image, target_h, target_w);
}

std::vector<float> ImageProcessor::preprocess_from_file(
    const std::string& image_path,
    int target_h,
    int target_w
) {
    // 从文件读取图像
    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
    
    if (image.empty()) {
        throw std::runtime_error("Failed to read image from: " + image_path);
    }
    
    std::cout << "[ImageProcessor] Loaded image: " 
              << image.cols << "x" << image.rows << std::endl;
    
    return process_image(image, target_h, target_w);
}

std::vector<float> ImageProcessor::process_image(
    const cv::Mat& image,
    int target_h,
    int target_w
) {
    // 2. Resize到目标尺寸
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(target_w, target_h));
    std::cout << "[ImageProcessor] Resized to: " 
              << target_w << "x" << target_h << std::endl;
    
    // 3. BGR -> RGB
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    
    // 4. 归一化 [0, 255] -> [0, 1]
    cv::Mat normalized;
    resized.convertTo(normalized, CV_32F, 1.0 / 255.0);
    
    // 5. HWC -> CHW格式（YOLO要求的输入格式）
    std::vector<cv::Mat> channels(3);
    cv::split(normalized, channels);
    
    std::vector<float> result;
    result.reserve(3 * target_h * target_w);
    
    // 按C-H-W顺序排列：R通道 -> G通道 -> B通道
    for (int c = 0; c < 3; c++) {
        // 注意：OpenCV split后顺序是BGR，但我们已经转成了RGB
        // 所以channels[0]=R, channels[1]=G, channels[2]=B
        result.insert(
            result.end(),
            (float*)channels[c].data,
            (float*)channels[c].data + target_h * target_w
        );
    }
    
    std::cout << "[ImageProcessor] Final tensor shape: [1, 3, " 
              << target_h << ", " << target_w << "]" << std::endl;
    std::cout << "[ImageProcessor] Total elements: " << result.size() << std::endl;
    
    return result;
}