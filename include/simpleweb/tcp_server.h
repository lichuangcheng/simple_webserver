#ifndef YOLANDAPP_TCP_SERVER_H
#define YOLANDAPP_TCP_SERVER_H

#include "simpleweb/acceptor.h"
#include "simpleweb/event_loop.h"
#include "simpleweb/event_loop_thread_pool.h"
#include "simpleweb/buffer.h"
#include "simpleweb/tcp_connection.h"


namespace simpleweb {
    

class TCPServer 
{
public:
    TCPServer(EventLoop *ev_loop, int port, int thread_num = 0);
    
    TCPServer(EventLoop *ev_loop, int port,
                connection_completed_call_back connectionCallBack,
                message_call_back messageCallBack,
                write_completed_call_back writeCompletedCallBack,
                connection_closed_call_back connectionClosedCallBack,
                int threadNum);
    
    ~TCPServer() = default;

    TCPServer& on_connection_completed(connection_completed_call_back fn);
    TCPServer& on_message(message_call_back fn);
    TCPServer& on_write_completed(write_completed_call_back fn);
    TCPServer& on_connection_close(connection_closed_call_back fn);

    //开启监听
    void start();

    //设置callback数据
    void set_data(void * data);

    friend class HttpServer;
protected:
    virtual std::shared_ptr<TCPConnection> create_connection(int conn_fd, EventLoop *loop);

    void handle_connection_established();
    
private:
    EventLoop *eventLoop;
    Acceptor acceptor;

    connection_completed_call_back connectionCompletedCallBack;
    message_call_back messageCallBack;
    write_completed_call_back writeCompletedCallBack;
    connection_closed_call_back connectionClosedCallBack;
    std::unique_ptr<EventLoopThreadPool> threadPool;

    void * data {nullptr}; //for callback use: http_server
};


/// 
/// inlines
///
inline TCPServer& TCPServer::on_connection_completed(connection_completed_call_back fn) 
{
    connectionCompletedCallBack = fn;
    return *this;
}


inline TCPServer& TCPServer::on_message(message_call_back fn) 
{
    messageCallBack = fn;
    return *this;
}


inline TCPServer& TCPServer::on_write_completed(write_completed_call_back fn) 
{
    writeCompletedCallBack = fn;
    return *this;
}


inline TCPServer& TCPServer::on_connection_close(connection_closed_call_back fn) 
{
    connectionClosedCallBack = fn;
    return *this;
}


} // namespace simpleweb


#endif
