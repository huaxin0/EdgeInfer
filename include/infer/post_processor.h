#pragma once
#include <vector>
#include <string>

/**
 * @brief 检测结果结构
 */
struct Detection {
    int class_id;
    std::string class_name;
    float confidence;
    float x, y, w, h;  // bbox坐标 (center_x, center_y, width, height)
};

/**
 * @brief YOLO后处理器
 */
class PostProcessor {
public:
    /**
     * @brief 处理YOLOv8输出
     * @param raw_output 原始输出 [1, 84, 8400]
     * @param conf_thresh 置信度阈值
     * @param iou_thresh NMS的IoU阈值
     * @param input_h 输入图像高度
     * @param input_w 输入图像宽度
     * @return 检测结果列表
     */
    static std::vector<Detection> processYOLOv8(
        const std::vector<float>& raw_output,
        float conf_thresh = 0.5f,
        float iou_thresh = 0.45f,
        int input_h = 640,
        int input_w = 640
    );
    
private:
    /**
     * @brief NMS（非极大值抑制）
     */
    static std::vector<Detection> nms(
        std::vector<Detection>& detections,
        float iou_thresh
    );
    
    /**
     * @brief 计算两个bbox的IoU
     */
    static float computeIoU(const Detection& a, const Detection& b);
    
    /**
     * @brief COCO类别名称
     */
    static const std::vector<std::string> COCO_CLASSES;
};