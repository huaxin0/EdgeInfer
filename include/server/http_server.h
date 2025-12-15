#pragma once
//定义一个类，初始化服务端口号， 定义开始函数
//头文件在同一个编译单元中只被包含一次，防止重复定义
//变量名使用 port_（下划线后缀）是常见的命名约定，用于区分成员变量和局部变量
#include  "httplib.h"
#include <memory>
class HttpServer{
public:
    HttpServer(int port =50003 );  //构造函数
    void start();
private:
    int port_;  //私有访问权限说明符，后面的成员都是私有的，只能由类内部的成员函数访问
    httplib::Server server;   // ← 关键：声明 server
};
