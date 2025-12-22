#include "server/http_server.h"
#include "infer/infer_manager.h"
#include <iostream>

HttpServer::HttpServer() {
    server_ = std::make_unique<httplib::Server>();  //服务端
    infer_mgr_ = std::make_shared<InferManager>();  //推理端

    server_->Post("/infer", [this](const httplib::Request& req,
                                   httplib::Response& res) {

        //校验 Content-Type
        auto content_type =req.get_header_value("Content-Type");  //校验应用端给的字段
        if(content_type.find("application/json")==std::string::npos) {  //表示条件成立并没找到这个字段
            res.status=415;
            res.set_content(
                R"({"code":415,"msg":"Content-Type must be application/json"})",
            "application/json"
            );
            return;
        }                           

        // 1️⃣ 校验 Body 是否为空
        if (req.body.empty()) {
            res.status = 400;
            res.set_content(
            R"({"code":400,"msg":"empty request body"})",
            "application/json"
        );
        return;
        }

    // 2️⃣ 校验 Body 大小（例如限制 1MB）
        constexpr size_t kMaxBodySize = 1024 * 1024;
        if (req.body.size() > kMaxBodySize) {
            res.status = 413;  // Payload Too Large
            res.set_content(
            R"({"code":413,"msg":"request body too large"})",
            "application/json"
        );
        return;
        }                            
        std::cout << "[HTTP] /infer called" << std::endl;
        std::cout << "Request body: " << req.body << std::endl;

        std::string result = infer_mgr_->infer(req.body);

        res.set_content(
            R"({"code":0,"msg":"ok","result":")" + result + R"("})",
            "application/json"
        );
    });
}

void HttpServer::start(int port) {
    std::cout << "HTTP server listen on port " << port << std::endl;
    server_->listen("0.0.0.0", port);
    //0.0.0.0 表示监听所有可用的网络接口
}
