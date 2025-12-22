#pragma once
#include <vector>
#include <string>
#include <opencv2/opencv.hpp> //调用opencv库

class ImageProcessor {
public:
    /**
     * @brief 从字节数组解码图像并预处理
     * @param image_data 图像字节数组（JPEG/PNG格式）
     * @param target_h 目标高度
     * @param target_w 目标宽度
     * @return 预处理后的float数组，格式为CHW (C, H, W)，归一化到[0,1]
     * @throws std::runtime_error 如果解码或预处理失败
     */
    static std::vector<float> preprocess(
        const std::vector<uint8_t>& image_data,
        int target_h = 640,
        int target_w = 640
    );
    
    /**
     * @brief 从文件路径读取图像并预处理（用于调试）
     * @param image_path 图像文件路径
     * @param target_h 目标高度
     * @param target_w 目标宽度
     * @return 预处理后的float数组
     */
    static std::vector<float> preprocess_from_file(
        const std::string& image_path,
        int target_h = 640,
        int target_w = 640
    );

private:
    /**
     * @brief 核心预处理逻辑
     * @param image 输入的cv::Mat图像
     * @param target_h 目标高度
     * @param target_w 目标宽度
     * @return 预处理后的float数组
     */
    static std::vector<float> process_image(
        const cv::Mat& image,
        int target_h,
        int target_w
    );
};