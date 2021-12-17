#include <simpleweb/http_server.h>

using namespace simpleweb;

int main(int argc, char const *argv[])
{
    HttpServer svr;
    svr.listen(8080);
    return 0;
}

