#if !defined(SIMPLEWEB_HTTP_SERVER_INCLUDED)
#define SIMPLEWEB_HTTP_SERVER_INCLUDED


#include "simpleweb/http_request.h"
#include "simpleweb/http_response.h"
#include "simpleweb/thread_pool.h"


namespace simpleweb {
    

class HttpServer
{
public:
    HttpServer();
    ~HttpServer();

    void set_root(std::string root);
    bool listen(int port);
private:
    int sock_ {-1};
    thread_pool tp_;
    std::string root_;
};


///
/// inline 
///
inline void HttpServer::set_root(std::string root) 
{
    root_ = std::move(root);
}


} // namespace simpleweb


#endif // SIMPLEWEB_HTTP_SERVER_INCLUDED
