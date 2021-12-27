#include <assert.h>
#include <sys/fcntl.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "simpleweb/acceptor.h"
#include "simpleweb/log.h"
#include "simpleweb/tcp_server.h"
#include "simpleweb/thread_pool.h"
#include "simpleweb/event_loop.h"


namespace simpleweb {


TCPServer::TCPServer(EventLoop *eventLoop, int port, int thread_num) 
    : TCPServer(nullptr, eventLoop, port, thread_num)
{
}


TCPServer::TCPServer(TCPConnectionFactory::Ptr factory, EventLoop *eventLoop, int port, int threadNum)
    : eventLoop(eventLoop)
    , acceptor(port)
    , factory_(std::move(factory))
    , threadPool(new EventLoopThreadPool(eventLoop, threadNum))
{
}


void TCPServer::handle_connection_established() 
{
    std::string ip;
    uint16_t port;
    auto conn = acceptor.accept(ip, port);
   
    make_nonblocking(conn);
    yolanda_msgx("new connection established, socket == %d", conn);

    // choose event loop from the thread pool
    auto *eventLoop = threadPool->get_loop();

    // create a new tcp connection
    auto tcp_conn = factory_->create_connection(conn, eventLoop);
    //for callback use
    if (data)
        tcp_conn->data = data;

    // add event read for the new connection
    eventLoop->add_channel(tcp_conn);
}


//开启监听
void TCPServer::start() 
{
    //开启多个线程
    threadPool->start();

    //acceptor主线程， 同时把tcpServer作为参数传给channel对象
    auto channel = std::make_shared<ChannelAdaptor>(acceptor.socket_fd(), EPOLLIN, eventLoop);
    channel->read(&TCPServer::handle_connection_established, this);
    
    eventLoop->add_channel(channel);
}


//设置callback数据
void TCPServer::set_data(void *data) 
{
    if (data != nullptr) {
        this->data = data;
    }
}

} // namespace simpelweb

