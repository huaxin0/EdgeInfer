

🚀 **Edge AI Inference Engine Based on Ascend NPU**

A high-performance AI inference service built on Huawei Ascend NPU, designed for efficient object detection on edge devices.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Ascend%20310-orange.svg)](https://www.hiascend.com/)

---

## ✨ Features

- 🔥 **High Performance**: 14ms inference latency on Ascend 310B1
- 🎯 **YOLOv8 Support**: State-of-the-art object detection
- 🌐 **HTTP API**: RESTful JSON interface with Base64 image encoding
- 🏗️ **Modular Design**: Clean architecture with hardware abstraction layer
- 📦 **Easy Deployment**: Single binary with minimal dependencies
- 🔧 **Extensible**: Easy to add new models and backends

---

## 📊 Performance

| Metric | Value |
|--------|-------|
| NPU Inference Time | 14ms |
| End-to-End Latency | 60-70ms |
| Input Resolution | 640x640 |
| Throughput | ~60 FPS (single stream) |

**Latency Breakdown**:
- Base64 Decoding: ~5ms
- Image Preprocessing: ~20ms
- NPU Inference: **14ms**
- Post-processing (NMS): ~10ms
- JSON Serialization: ~5ms
- Network Overhead: ~13ms

---

## 🛠️ Tech Stack

**Core Technologies**:
- **C++17**: Modern C++ with smart pointers and STL
- **Ascend ACL**: Huawei's AI Computing Language for NPU inference
- **OpenCV 4.5**: Image processing and computer vision
- **cpp-httplib**: Lightweight HTTP server library
- **nlohmann/json**: Modern JSON library for C++

**AI Framework**:
- **YOLOv8**: Ultralytics' latest object detection model
- **ONNX**: Model interchange format
- **Ascend OM**: Optimized model format for Ascend NPU

---

## 🏗️ Architecture
```
┌─────────────────────────────────────────────────────────┐
│                      HTTP Server                         │
│                  (JSON RESTful API)                      │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│                   InferManager                           │
│        (Business Logic & Orchestration)                  │
│  ┌──────────┐  ┌──────────┐  ┌────────────────┐        │
│  │ Base64   │  │  Image   │  │     Post       │        │
│  │ Decoder  │  │Processor │  │  Processor     │        │
│  └──────────┘  └──────────┘  └────────────────┘        │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│                  AscendInfer                             │
│          (Hardware Abstraction Layer)                    │
│  ┌─────────────────────────────────────────────┐        │
│  │  ACL Runtime │ Model Loading │ Memory Mgmt  │        │
│  └─────────────────────────────────────────────┘        │
└────────────────────┬────────────────────────────────────┘
                     │
              ┌──────▼──────┐
              │ Ascend NPU  │
              │  (310B1)    │
              └─────────────┘
```

**Design Principles**:
- **Layered Architecture**: Separation of concerns between business logic and hardware
- **Dependency Inversion**: Hardware-agnostic inference interface
- **Single Responsibility**: Each class has one clear purpose
- **Easy Testing**: Modular components can be tested independently

---

## 🚀 Quick Start

### Prerequisites

**Hardware**:
- Huawei Ascend 310/310P NPU

**Software**:
- Ubuntu 20.04/22.04 (aarch64)
- CANN Toolkit 8.0+
- OpenCV 4.x
- CMake 3.10+
- GCC 7.5+

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/yourusername/EdgeInfer.git
cd EdgeInfer
```

2. **Install dependencies**
```bash
# OpenCV
sudo apt install libopencv-dev

# CANN Toolkit (if not installed)
# Download from: https://www.hiascend.com/software/cann
```

3. **Prepare the model**
```bash
# Download YOLOv8s ONNX model
cd models/onnx
wget https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8s.onnx

# Convert to Ascend OM format
cd ..
source /usr/local/Ascend/ascend-toolkit/set_env.sh
atc --model=onnx/yolov8s.onnx \
    --framework=5 \
    --output=om/yolov8s \
    --input_format=NCHW \
    --input_shape="images:1,3,640,640" \
    --soc_version=Ascend310B1
```

4. **Build the project**
```bash
mkdir build && cd build
cmake ..
make -j4
```

5. **Run the service**
```bash
./edgeinfer
# Server starts on http://0.0.0.0:50003
```

---

## 📖 API Documentation

### Inference Endpoint

**URL**: `POST /infer`

**Request**:
```json
{
  "image": "<base64_encoded_image>",
  "model": "yolov8s",
  "conf_thresh": 0.5,
  "iou_thresh": 0.45
}
```

**Response**:
```json
{
  "code": 0,
  "msg": "success",
  "detections": [
    {
      "class_id": 0,
      "class_name": "person",
      "confidence": 0.95,
      "bbox": [100.0, 200.0, 50.0, 100.0]
    }
  ],
  "latency_ms": 62,
  "npu_latency_ms": 14,
  "image_size": [640, 640]
}
```

**Parameters**:
- `image` (required): Base64-encoded JPEG/PNG image
- `model` (optional): Model name, default: "yolov8s"
- `conf_thresh` (optional): Confidence threshold, default: 0.5
- `iou_thresh` (optional): NMS IoU threshold, default: 0.45

**bbox format**: `[center_x, center_y, width, height]`

### Example Usage

**Using curl**:
```bash
# Encode image to base64
IMAGE_B64=$(base64 -w 0 test.jpg)

# Send request
curl -X POST http://localhost:50003/infer \
  -H "Content-Type: application/json" \
  -d "{
    \"image\": \"$IMAGE_B64\",
    \"conf_thresh\": 0.5
  }"
```

**Using Python**:
```python
import requests
import base64

# Read and encode image
with open("test.jpg", "rb") as f:
    image_b64 = base64.b64encode(f.read()).decode()

# Send request
response = requests.post(
    "http://localhost:50003/infer",
    json={
        "image": image_b64,
        "conf_thresh": 0.5
    }
)

result = response.json()
print(f"Detected {len(result['detections'])} objects")
```

---

## 📁 Project Structure
```
EdgeInfer/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── include/                    # Header files
│   ├── infer/
│   │   ├── infer_manager.h     # Main inference orchestrator
│   │   ├── ascend_infer.h      # Ascend NPU inference engine
│   │   └── post_processor.h    # YOLO post-processing
│   ├── server/
│   │   └── http_server.h       # HTTP server
│   └── utils/
│       ├── base64.h            # Base64 decoder
│       └── image_processor.h   # Image preprocessing
├── src/                        # Source files
│   ├── main.cpp
│   ├── infer/
│   │   ├── infer_manager.cpp
│   │   ├── ascend_infer.cpp
│   │   └── post_processor.cpp
│   ├── server/
│   │   └── http_server.cpp
│   └── utils/
│       ├── base64.cpp
│       └── image_processor.cpp
├── models/                     # Model files
│   ├── onnx/                   # ONNX models
│   │   └── yolov8s.onnx
│   └── om/                     # Ascend OM models
│       └── yolov8s.om
├── third_party/                # Third-party libraries
│   └── httplib.h               # cpp-httplib header
└── test_data/                  # Test images
    └── test.jpg
```

---

## 🔬 Technical Details

### Image Preprocessing Pipeline
```cpp
Input: JPEG/PNG bytes
  ↓
cv::imdecode           // Decode to cv::Mat
  ↓
cv::resize(640, 640)   // Resize to model input size
  ↓
BGR → RGB              // Color space conversion
  ↓
Normalize [0, 255] → [0, 1]  // Normalization
  ↓
HWC → CHW              // Channel-first format
  ↓
Output: float[1,3,640,640]  // Ready for NPU
```

### YOLOv8 Post-processing

**Output Format**: `[1, 84, 8400]`
- 8400 proposals from 3 feature levels
- 84 dimensions: 4 (bbox) + 80 (classes)
- No objectness score (anchor-free design)

**NMS Algorithm**:
1. Sort proposals by confidence (descending)
2. Select highest confidence box
3. Calculate IoU with remaining boxes
4. Suppress boxes with IoU > threshold
5. Repeat until all boxes processed

### ACL Inference Pipeline
```cpp
1. aclInit()                    // Initialize ACL
2. aclrtSetDevice()             // Set device
3. aclrtCreateContext()         // Create context
4. aclmdlLoadFromFile()         // Load OM model
5. aclrtMalloc()                // Allocate device memory
6. aclrtMemcpy(H2D)             // Copy input to device
7. aclmdlExecute()              // Execute inference
8. aclrtMemcpy(D2H)             // Copy output to host
9. Cleanup resources
```

---

## 🐛 Troubleshooting

### Common Issues

**1. ACL initialization failed**
```
Error: aclInit failed: 500000
Solution: Check CANN toolkit installation and environment variables
```

**2. Model loading failed**
```
Error: aclmdlLoadFromFile failed: 507002
Solution: Verify model file path and soc_version matches your device
```

**3. Memory allocation failed**
```
Error: Malloc input buffer failed: 107002
Solution: Ensure aclrtSetCurrentContext() is called before malloc
```

**4. OpenCV not found**
```
Error: opencv2/opencv.hpp: No such file or directory
Solution: Install OpenCV development package
```

---

## 🗺️ Roadmap

### Current Features (v0.1)
- [x] Basic HTTP inference service
- [x] YOLOv8 object detection
- [x] Ascend NPU support
- [x] JSON API

### Planned Features
- [ ] Dynamic batch processing
- [ ] Model hot-swapping
- [ ] Multi-model support
- [ ] Video stream inference
- [ ] Prometheus metrics
- [ ] Docker deployment
- [ ] Performance benchmarks
- [ ] Unit tests

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

**Development Setup**:
```bash
git clone https://github.com/yourusername/EdgeInfer.git
cd EdgeInfer
# Follow Quick Start guide
```

**Code Style**:
- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use meaningful variable names
- Add comments for complex logic

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- [Ultralytics YOLOv8](https://github.com/ultralytics/ultralytics) - YOLO implementation
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - HTTP server library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON library
- [OpenCV](https://opencv.org/) - Computer vision library
- [Huawei Ascend](https://www.hiascend.com/) - NPU hardware and software stack

---

## 📬 Contact

- GitHub: [@huaxin0](https://github.com/huaxin0)
- Email:19946720495@163.com

---

## 📊 Citation

If you use EdgeInfer in your research, please cite:
```bibtex
@software{edgeinfer2025,
  author = {huaxin0},
  title = {EdgeInfer: Edge AI Inference Engine Based on Ascend NPU},
  year = {2025},
  url = {https://github.com/huaxin0/EdgeInfer}
}
```

---

**⭐ Star this repo if you find it helpful!**




