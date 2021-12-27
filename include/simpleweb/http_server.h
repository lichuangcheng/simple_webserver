#if !defined(SIMPLEWEB_HTTP_SERVER_INCLUDED)
#define SIMPLEWEB_HTTP_SERVER_INCLUDED


#include "simpleweb/http_request.h"
#include "simpleweb/http_response.h"
#include "simpleweb/thread_pool.h"
#include "simpleweb/tcp_server.h"


namespace simpleweb {


using request_callback = std::function<void (HttpRequest &req, HttpResponse &res)>;


class HttpServer
{
public:
    HttpServer(EventLoop *eventLoop, int port, int thread_num = 0);
        /// 根据给定的端口及线程数目初始化 HttpServer

    ~HttpServer();
        /// 析构

    HttpServer& request(request_callback request_process);
        /// 设置 http 请求回调，在发生http请求时，将会调用 request_process
        /// 将会返回当前对象引用

    void start();
        /// 启动 http server, 启动后，需启动绑定的 EventLoop 进入事件循环;

    void set_root(std::string root);
        /// 设置根目录路劲

private:
    std::string root_;
    TCPServer tcp_svr_;
    request_callback request_fn_;
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
