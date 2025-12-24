#pragma once
#include <string>
#include <vector>
#include <memory>
#include "acl/acl.h"

/**
 * @brief 昇腾推理引擎
 */
class AscendInfer {
public:
    AscendInfer(int device_id = 0);
    ~AscendInfer();
    
    /**
     * @brief 初始化ACL和设备
     */
    bool init();
    
    /**
     * @brief 加载OM模型
     * @param model_path OM模型路径
     */
    bool loadModel(const std::string& model_path);
    
    /**
     * @brief 执行推理
     * @param input_data 输入数据（float数组，已预处理）
     * @return 输出数据（float数组）
     */
    std::vector<float> infer(const std::vector<float>& input_data);
    
    /**
     * @brief 获取输入大小
     */
    size_t getInputSize() const { return input_size_; }
    
    /**
     * @brief 获取输出大小
     */
    size_t getOutputSize() const { return output_size_; }
    
private:
    int device_id_;
    aclrtContext context_;
    aclrtStream stream_;
    
    uint32_t model_id_;
    aclmdlDesc* model_desc_;
    aclmdlDataset* input_dataset_;
    aclmdlDataset* output_dataset_;
    
    void* input_device_buffer_;
    void* output_device_buffer_;
    
    size_t input_size_;
    size_t output_size_;
    
    bool initialized_;
    bool model_loaded_;
    
    // 辅助函数
    void cleanup();
    bool createInputDataset(const std::vector<float>& input_data);
    bool createOutputDataset();
    void destroyDataset(aclmdlDataset** dataset);
};