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


TCPServer::TCPServer(EventLoop *ev_loop, int port, int thread_num) 
    : TCPServer(nullptr, ev_loop, port, thread_num)
{
}


TCPServer::TCPServer(TCPConnectionFactory::Ptr factory, EventLoop *ev_loop, int port, int thread_num)
    : ev_loop_(ev_loop)
    , acceptor_(port)
    , factory_(std::move(factory))
    , evtp_(new EventLoopThreadPool(ev_loop, thread_num))
{
}


void TCPServer::handle_connection_established() 
{
    std::string ip;
    uint16_t port;
    auto conn = acceptor_.accept(ip, port);
   
    make_nonblocking(conn);
    yolanda_msgx("new connection established, socket == %d", conn);

    // choose event loop from the thread pool
    auto *ev_loop = evtp_->get_loop();

    // create a new tcp connection
    auto tcp_conn = factory_->create_connection(conn, ev_loop);

    // add event read for the new connection
    ev_loop->add_channel(tcp_conn);
}


void TCPServer::start() 
{
    // 开启多个线程
    evtp_->start();

    // acceptor主线程, 监听连接到来
    auto channel = std::make_shared<ChannelAdaptor>(acceptor_.socket_fd(), EPOLLIN, ev_loop_);
    channel->read(&TCPServer::handle_connection_established, this);
    
    ev_loop_->add_channel(channel);
}


} // namespace simpelweb

