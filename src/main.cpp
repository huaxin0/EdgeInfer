#include <iostream>
#include "server/http_server.h"
using namespace std;
int main()
{
    cout<<"EdgeInfer start ...."<<endl;
     HttpServer server(50003);
    server.start();
    return 0;
}