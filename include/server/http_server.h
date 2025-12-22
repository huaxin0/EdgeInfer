#pragma once     // 防止头文件被重复包含
//定义一个类，初始化服务端口号， 定义开始函数
//变量名使用 port_（下划线后缀）是常见的命名约定，用于区分成员变量和局部变量
#include  "httplib.h"
#include <memory>    // 智能指针相关头文件






class InferManager; //声明这个库可调用


class HttpServer{
public:
    HttpServer();  //构造函数
    void start(int port);
private:
    int port_;  //私有访问权限说明符，后面的成员都是私有的，只能由类内部的成员函数访问
   // httplib::Server server;   // ← 关键：服务器实例,声明 server   

    std::unique_ptr<httplib::Server> server_;   //创建服务端智能指针
    std::shared_ptr<InferManager> infer_mgr_;   //创建推理智能指针
};
