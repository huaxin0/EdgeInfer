#include "server/http_server.h"
#include  <iostream>

HttpServer::HttpServer(int port)
    :port_(port){}

void HttpServer::start(){
    server.Get("/ping", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("pong", "text/plain");
    });

    std::cout << "HTTP server listening on port " << port_ << std::endl;
    server.listen("0.0.0.0", port_);
}