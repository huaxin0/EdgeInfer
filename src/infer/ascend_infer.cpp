#include "infer/ascend_infer.h"
#include <iostream>
#include <cstring>

AscendInfer::AscendInfer(int device_id)
    : device_id_(device_id),
      context_(nullptr),
      stream_(nullptr),
      model_id_(0),
      model_desc_(nullptr),
      input_dataset_(nullptr),
      output_dataset_(nullptr),
      input_device_buffer_(nullptr),
      output_device_buffer_(nullptr),
      input_size_(0),
      output_size_(0),
      initialized_(false),
      model_loaded_(false) {
}

AscendInfer::~AscendInfer() {
    cleanup();
}

bool AscendInfer::init() {
    if (initialized_) {
        std::cout << "[AscendInfer] Already initialized" << std::endl;
        return true;
    }
    
    std::cout << "[AscendInfer] Initializing ACL..." << std::endl;
    
    // 1. 初始化ACL
    aclError ret = aclInit(nullptr);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclInit failed: " << ret << std::endl;
        return false;
    }
    
    // 2. 设置设备
    ret = aclrtSetDevice(device_id_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclrtSetDevice failed: " << ret << std::endl;
        aclFinalize();
        return false;
    }
    
    // 3. 创建Context
    ret = aclrtCreateContext(&context_, device_id_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclrtCreateContext failed: " << ret << std::endl;
        aclrtResetDevice(device_id_);
        aclFinalize();
        return false;
    }
    
    // 4. 创建Stream
    ret = aclrtCreateStream(&stream_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclrtCreateStream failed: " << ret << std::endl;
        aclrtDestroyContext(context_);
        aclrtResetDevice(device_id_);
        aclFinalize();
        return false;
    }
    
    initialized_ = true;
    std::cout << "[AscendInfer] ACL initialized successfully" << std::endl;
    return true;
}

bool AscendInfer::loadModel(const std::string& model_path) {
    if (!initialized_) {
        std::cerr << "[AscendInfer] Not initialized, call init() first" << std::endl;
        return false;
    }
    
    if (model_loaded_) {
        std::cout << "[AscendInfer] Model already loaded" << std::endl;
        return true;
    }
    
    std::cout << "[AscendInfer] Loading model: " << model_path << std::endl;
    
    // 设置当前Context
    aclError ret = aclrtSetCurrentContext(context_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclrtSetCurrentContext failed: " << ret << std::endl;
        return false;
    }
    
    // 1. 加载模型
    ret = aclmdlLoadFromFile(model_path.c_str(), &model_id_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclmdlLoadFromFile failed: " << ret << std::endl;
        return false;
    }
    
    std::cout << "[AscendInfer] Model loaded, ID: " << model_id_ << std::endl;
    
    // 2. 获取模型描述
    model_desc_ = aclmdlCreateDesc();
    ret = aclmdlGetDesc(model_desc_, model_id_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclmdlGetDesc failed: " << ret << std::endl;
        aclmdlUnload(model_id_);
        return false;
    }
    
    // 3. 获取输入输出大小
    input_size_ = aclmdlGetInputSizeByIndex(model_desc_, 0);
    output_size_ = aclmdlGetOutputSizeByIndex(model_desc_, 0);
    
    std::cout << "[AscendInfer] Input size: " << input_size_ 
              << " bytes (" << input_size_/1024/1024 << " MB)" << std::endl;
    std::cout << "[AscendInfer] Output size: " << output_size_ 
              << " bytes (" << output_size_/1024/1024 << " MB)" << std::endl;
    
    // 4. 分配输出缓冲区（输入每次推理时分配）
    ret = aclrtMalloc(&output_device_buffer_, output_size_, ACL_MEM_MALLOC_NORMAL_ONLY);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] Malloc output buffer failed: " << ret << std::endl;
        aclmdlDestroyDesc(model_desc_);
        aclmdlUnload(model_id_);
        return false;
    }
    
    model_loaded_ = true;
    std::cout << "[AscendInfer] ✅ Model loaded successfully" << std::endl;
    return true;
}

bool AscendInfer::createInputDataset(const std::vector<float>& input_data) {
    // 检查输入大小
    size_t expected_size = input_size_ / sizeof(float);
    if (input_data.size() != expected_size) {
        std::cerr << "[AscendInfer] Input size mismatch: expected " 
                  << expected_size << ", got " << input_data.size() << std::endl;
        return false;
    }
    
    // 1. 分配设备内存
    aclError ret = aclrtMalloc(&input_device_buffer_, input_size_, ACL_MEM_MALLOC_NORMAL_ONLY);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] Malloc input buffer failed: " << ret << std::endl;
        return false;
    }
    
    // 2. 拷贝数据到设备
    ret = aclrtMemcpy(input_device_buffer_, input_size_,
                     input_data.data(), input_size_,
                     ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] Memcpy H2D failed: " << ret << std::endl;
        aclrtFree(input_device_buffer_);
        input_device_buffer_ = nullptr;
        return false;
    }
    
    // 3. 创建Dataset
    input_dataset_ = aclmdlCreateDataset();
    
    // 4. 创建DataBuffer
    aclDataBuffer* input_data_buffer = aclCreateDataBuffer(input_device_buffer_, input_size_);
    if (input_data_buffer == nullptr) {
        std::cerr << "[AscendInfer] Create input data buffer failed" << std::endl;
        aclrtFree(input_device_buffer_);
        input_device_buffer_ = nullptr;
        return false;
    }
    
    // 5. 添加到Dataset
    ret = aclmdlAddDatasetBuffer(input_dataset_, input_data_buffer);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] Add input dataset buffer failed: " << ret << std::endl;
        aclDestroyDataBuffer(input_data_buffer);
        aclrtFree(input_device_buffer_);
        input_device_buffer_ = nullptr;
        return false;
    }
    
    return true;
}

bool AscendInfer::createOutputDataset() {
    // 1. 创建Dataset
    output_dataset_ = aclmdlCreateDataset();
    
    // 2. 创建DataBuffer
    aclDataBuffer* output_data_buffer = aclCreateDataBuffer(output_device_buffer_, output_size_);
    if (output_data_buffer == nullptr) {
        std::cerr << "[AscendInfer] Create output data buffer failed" << std::endl;
        return false;
    }
    
    // 3. 添加到Dataset
    aclError ret = aclmdlAddDatasetBuffer(output_dataset_, output_data_buffer);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] Add output dataset buffer failed: " << ret << std::endl;
        aclDestroyDataBuffer(output_data_buffer);
        return false;
    }
    
    return true;
}

std::vector<float> AscendInfer::infer(const std::vector<float>& input_data) {
    if (!model_loaded_) {
        std::cerr << "[AscendInfer] Model not loaded" << std::endl;
        return {};
    }
    
    // 设置当前Context
    aclError ret = aclrtSetCurrentContext(context_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclrtSetCurrentContext failed: " << ret << std::endl;
        return {};
    }
    
    std::cout << "[AscendInfer] Starting inference..." << std::endl;
    
    // 1. 创建输入Dataset
    if (!createInputDataset(input_data)) {
        std::cerr << "[AscendInfer] Create input dataset failed" << std::endl;
        return {};
    }
    
    // 2. 创建输出Dataset
    if (!createOutputDataset()) {
        std::cerr << "[AscendInfer] Create output dataset failed" << std::endl;
        destroyDataset(&input_dataset_);
        if (input_device_buffer_ != nullptr) {
            aclrtFree(input_device_buffer_);
            input_device_buffer_ = nullptr;
        }
        return {};
    }
    
    // 3. 执行推理
    ret = aclmdlExecute(model_id_, input_dataset_, output_dataset_);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] aclmdlExecute failed: " << ret << std::endl;
        destroyDataset(&input_dataset_);
        destroyDataset(&output_dataset_);
        if (input_device_buffer_ != nullptr) {
            aclrtFree(input_device_buffer_);
            input_device_buffer_ = nullptr;
        }
        return {};
    }
    
    std::cout << "[AscendInfer]  Inference completed" << std::endl;
    
    // 4. 获取输出数据
    size_t output_elem_count = output_size_ / sizeof(float);
    std::vector<float> output_data(output_elem_count);
    
    ret = aclrtMemcpy(output_data.data(), output_size_,
                     output_device_buffer_, output_size_,
                     ACL_MEMCPY_DEVICE_TO_HOST);
    if (ret != ACL_SUCCESS) {
        std::cerr << "[AscendInfer] Memcpy D2H failed: " << ret << std::endl;
        destroyDataset(&input_dataset_);
        destroyDataset(&output_dataset_);
        if (input_device_buffer_ != nullptr) {
            aclrtFree(input_device_buffer_);
            input_device_buffer_ = nullptr;
        }
        return {};
    }
    
    std::cout << "[AscendInfer] Output copied, size: " << output_data.size() << std::endl;
    
    // 5. 清理本次推理的资源
    destroyDataset(&input_dataset_);
    destroyDataset(&output_dataset_);
    if (input_device_buffer_ != nullptr) {
        aclrtFree(input_device_buffer_);
        input_device_buffer_ = nullptr;
    }
    
    return output_data;
}

void AscendInfer::destroyDataset(aclmdlDataset** dataset) {
    if (dataset == nullptr || *dataset == nullptr) {
        return;
    }
    
    size_t buffer_count = aclmdlGetDatasetNumBuffers(*dataset);
    for (size_t i = 0; i < buffer_count; i++) {
        aclDataBuffer* data_buffer = aclmdlGetDatasetBuffer(*dataset, i);
        if (data_buffer != nullptr) {
            aclDestroyDataBuffer(data_buffer);
        }
    }
    
    aclmdlDestroyDataset(*dataset);
    *dataset = nullptr;
}

void AscendInfer::cleanup() {
    std::cout << "[AscendInfer] Cleaning up..." << std::endl;
    
    // 设置Context
    if (context_ != nullptr) {
        aclrtSetCurrentContext(context_);
    }
    
    // 清理输出缓冲区
    if (output_device_buffer_ != nullptr) {
        aclrtFree(output_device_buffer_);
        output_device_buffer_ = nullptr;
    }
    
    // 卸载模型
    if (model_loaded_ && model_desc_ != nullptr) {
        aclmdlDestroyDesc(model_desc_);
        model_desc_ = nullptr;
    }
    
    if (model_loaded_) {
        aclmdlUnload(model_id_);
        model_loaded_ = false;
    }
    
    // 销毁Stream和Context
    if (stream_ != nullptr) {
        aclrtDestroyStream(stream_);
        stream_ = nullptr;
    }
    
    if (context_ != nullptr) {
        aclrtDestroyContext(context_);
        context_ = nullptr;
    }
    
    // 重置设备
    if (initialized_) {
        aclrtResetDevice(device_id_);
        aclFinalize();
        initialized_ = false;
    }
    
    std::cout << "[AscendInfer] Cleanup completed" << std::endl;
}