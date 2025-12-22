#include "server/http_server.h"

int main() {
    HttpServer server;
    server.start(50003);
    return 0;
}
