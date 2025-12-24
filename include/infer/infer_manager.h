#pragma once
#include <string>
#include <vector>
#include <memory>
// 前向声明
class AscendInfer;

class InferManager
{  
public:
    InferManager();
    ~InferManager();
    /**
     * @brief 执行推理
     * @param input JSON格式的输入
     * @return JSON格式的输出
     */
    std::string infer(const std::string& input);

   

private:
    std::shared_ptr<AscendInfer> ascend_infer_;
    std::string model_path_;
};


