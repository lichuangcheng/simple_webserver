#ifndef YOLANDAPP_TCP_SERVER_H
#define YOLANDAPP_TCP_SERVER_H

#include "simpleweb/acceptor.h"
#include "simpleweb/event_loop.h"
#include "simpleweb/event_loop_thread_pool.h"
#include "simpleweb/buffer.h"
#include "simpleweb/tcp_connection.h"
#include "simpleweb/tcp_connection_factory.h"


namespace simpleweb {
    

class TCPServer 
{
public:
    TCPServer(EventLoop *ev_loop, int port, int thread_num = 0);

    TCPServer(TCPConnectionFactory::Ptr factory, EventLoop *ev_loop, int port, int thread_num = 0);
    
    ~TCPServer() = default;

    //开启监听
    void start();

    // 设置工厂方法
    void set_connection_factory(TCPConnectionFactory::Ptr factory);

    friend class HttpServer;

    TCPServer(const TCPServer&) = delete;
    TCPServer& operator = (const TCPServer&) = delete;
protected:
    void handle_connection_established();
    
private:
    EventLoop *ev_loop_;
    Acceptor acceptor_;

    TCPConnectionFactory::Ptr factory_;
    std::unique_ptr<EventLoopThreadPool> evtp_;
};


inline void TCPServer::set_connection_factory(TCPConnectionFactory::Ptr factory)
{
    factory_ = std::move(factory);
}

} // namespace simpleweb


#endif
