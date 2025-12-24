#include "infer/post_processor.h"
#include <algorithm>
#include <iostream>
#include <cmath>

// COCO 80类别名称
const std::vector<std::string> PostProcessor::COCO_CLASSES = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat",
    "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat",
    "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack",
    "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball",
    "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket",
    "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair",
    "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
    "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator",
    "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
};

std::vector<Detection> PostProcessor::processYOLOv8(
    const std::vector<float>& raw_output,
    float conf_thresh,
    float iou_thresh,
    int input_h,
    int input_w
) {
    std::cout << "[PostProcessor] Processing YOLOv8 output..." << std::endl;
    std::cout << "[PostProcessor] Raw output size: " << raw_output.size() << std::endl;
    
    // YOLOv8输出: [1, 84, 8400]
    const int num_proposals = 8400;
    const int num_classes = 80;
    const int bbox_dim = 4;
    
    // 验证输出大小
    if (raw_output.size() != 1 * 84 * 8400) {
        std::cerr << "[PostProcessor] Invalid output size: " << raw_output.size() << std::endl;
        return {};
    }
    
    std::vector<Detection> candidates;
    
    // 遍历8400个proposal
    for (int i = 0; i < num_proposals; i++) {
        // YOLOv8格式: [84, 8400]，需要按列读取
        // bbox: raw_output[0:4, i]
        // classes: raw_output[4:84, i]
        
        float cx = raw_output[0 * num_proposals + i];
        float cy = raw_output[1 * num_proposals + i];
        float w  = raw_output[2 * num_proposals + i];
        float h  = raw_output[3 * num_proposals + i];
        
        // 找最大类别置信度
        float max_class_score = 0.0f;
        int max_class_id = -1;
        
        for (int c = 0; c < num_classes; c++) {
            float class_score = raw_output[(4 + c) * num_proposals + i];
            if (class_score > max_class_score) {
                max_class_score = class_score;
                max_class_id = c;
            }
        }
        
        // 阈值过滤
        if (max_class_score < conf_thresh) {
            continue;
        }
        
        // 创建检测结果
        Detection det;
        det.class_id = max_class_id;
        det.class_name = (max_class_id < COCO_CLASSES.size()) 
                         ? COCO_CLASSES[max_class_id] 
                         : "unknown";
        det.confidence = max_class_score;
        det.x = cx;
        det.y = cy;
        det.w = w;
        det.h = h;
        
        candidates.push_back(det);
    }
    
    std::cout << "[PostProcessor] Candidates after threshold: " << candidates.size() << std::endl;
    
    // NMS
    std::vector<Detection> final_detections = nms(candidates, iou_thresh);
    
    std::cout << "[PostProcessor] Final detections after NMS: " << final_detections.size() << std::endl;
    
    return final_detections;
}

std::vector<Detection> PostProcessor::nms(
    std::vector<Detection>& detections,
    float iou_thresh
) {
    if (detections.empty()) {
        return {};
    }
    
    // 按置信度降序排序
    std::sort(detections.begin(), detections.end(),
              [](const Detection& a, const Detection& b) {
                  return a.confidence > b.confidence;
              });
    
    std::vector<Detection> result;
    std::vector<bool> suppressed(detections.size(), false);
    
    for (size_t i = 0; i < detections.size(); i++) {
        if (suppressed[i]) {
            continue;
        }
        
        result.push_back(detections[i]);
        
        // 抑制与当前框IoU过大的其他框
        for (size_t j = i + 1; j < detections.size(); j++) {
            if (suppressed[j]) {
                continue;
            }
            
            // 只对同类别的框做NMS
            if (detections[i].class_id != detections[j].class_id) {
                continue;
            }
            
            float iou = computeIoU(detections[i], detections[j]);
            if (iou > iou_thresh) {
                suppressed[j] = true;
            }
        }
    }
    
    return result;
}

float PostProcessor::computeIoU(const Detection& a, const Detection& b) {
    // 转换为 (x1, y1, x2, y2) 格式
    float a_x1 = a.x - a.w / 2;
    float a_y1 = a.y - a.h / 2;
    float a_x2 = a.x + a.w / 2;
    float a_y2 = a.y + a.h / 2;
    
    float b_x1 = b.x - b.w / 2;
    float b_y1 = b.y - b.h / 2;
    float b_x2 = b.x + b.w / 2;
    float b_y2 = b.y + b.h / 2;
    
    // 计算交集
    float inter_x1 = std::max(a_x1, b_x1);
    float inter_y1 = std::max(a_y1, b_y1);
    float inter_x2 = std::min(a_x2, b_x2);
    float inter_y2 = std::min(a_y2, b_y2);
    
    float inter_w = std::max(0.0f, inter_x2 - inter_x1);
    float inter_h = std::max(0.0f, inter_y2 - inter_y1);
    float inter_area = inter_w * inter_h;
    
    // 计算并集
    float a_area = a.w * a.h;
    float b_area = b.w * b.h;
    float union_area = a_area + b_area - inter_area;
    
    if (union_area <= 0) {
        return 0.0f;
    }
    
    return inter_area / union_area;
}