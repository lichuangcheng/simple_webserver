#if !defined(SIMPLEWEB_HTTP_SERVER_INCLUDED)
#define SIMPLEWEB_HTTP_SERVER_INCLUDED


#include "simpleweb/http_request.h"
#include "simpleweb/http_response.h"
#include "simpleweb/thread_pool.h"
#include "simpleweb/tcp_server.h"


namespace simpleweb {
    

class HttpServer : public TCPServer
{
public:
    using request_callback = std::function<int (HttpRequest *req, HttpResponse *res)>;

    HttpServer(EventLoop *eventLoop, int port, int thread_num = 0);

    ~HttpServer();

    void set_root(std::string root);

    request_callback request_fn;
private:
    std::string root_;
};


///
/// inlines
///
inline void HttpServer::set_root(std::string root) 
{
    root_ = std::move(root);
}


} // namespace simpleweb


#endif // SIMPLEWEB_HTTP_SERVER_INCLUDED
