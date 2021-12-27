#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "simpleweb/log.h"
#include "simpleweb/tcp_connection.h"
#include "simpleweb/buffer.h"
#include "simpleweb/event_loop.h"

namespace simpleweb {


void TCPConnection::read()
{
    if (input_buffer.socket_read(fd()) > 0) {
        //应用程序真正读取Buffer里的数据
        if (messageCallBack)
            messageCallBack(&input_buffer, this);
    } else {
        handle_connection_closed();
    }
}

void TCPConnection::write()
{
    loop_->assert_in_loop_thread();
    ssize_t nwrited = ::write(fd(), output_buffer.read_begin(), output_buffer.readable());
    if (nwrited > 0) {
        //已读nwrited字节
        output_buffer.readIndex += nwrited;
        //如果数据完全发送出去，就不需要继续了
        if (output_buffer.readable() == 0) {
            disable_write();
        }
        //回调writeCompletedCallBack
        if (writeCompletedCallBack) 
            writeCompletedCallBack(this);
    } else {
        yolanda_msgx("handle_write for tcp connection %s", name.c_str());
    }
}

void TCPConnection::error()
{
    perror("TCPConnection::error !!");
    handle_connection_closed();
}

void TCPConnection::close()
{
    printf("TCPConnection::close !!");
    handle_connection_closed();
}

void TCPConnection::handle_connection_closed() 
{
    if (connectionClosedCallBack)
        connectionClosedCallBack(this);

    loop_->remove_channel(shared_from_this());
}


TCPConnection::TCPConnection(int connected_fd, EventLoop *eventLoop,
                             connection_completed_call_back connectionCompletedCallBack,
                             connection_closed_call_back connectionClosedCallBack,
                             message_call_back messageCallBack, write_completed_call_back writeCompletedCallBack) 
    : Channel(connected_fd, EPOLLIN, eventLoop)
    , connectionCompletedCallBack(connectionCompletedCallBack)
    , connectionClosedCallBack(connectionClosedCallBack)
    , messageCallBack(messageCallBack)
    , writeCompletedCallBack(writeCompletedCallBack)
{
    char buf[64];
    sprintf(buf, "connection-%d", connected_fd);
    name = buf;

    if (connectionCompletedCallBack)
        connectionCompletedCallBack(this);
}

TCPConnection::~TCPConnection()
{
    if (fd() != -1)
        close_fd();
}

//应用层调用入口
int TCPConnection::send_data(void *data, size_t size) 
{
    ssize_t nwrited = 0;
    size_t nleft = size;
    int fault = 0;

    //先往套接字尝试发送数据
    if (!is_enabled_write() && output_buffer.readable() == 0) {
        nwrited = ::write(fd(), data, size);
        if (nwrited >= 0) {
            nleft = nleft - nwrited;
        } else {
            nwrited = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = 1;
                }
            }
        }
    }

    if (!fault && nleft > 0) {
        //拷贝到Buffer中，Buffer的数据由框架接管
        output_buffer.append(reinterpret_cast<uint8_t *>(data) + nwrited, nleft);
        if (!is_enabled_write()) {
            enable_write();
        }
    }

    return nwrited;
}

int TCPConnection::send_buffer(Buffer *buffer) {
    auto size = buffer->readable();
    int result = send_data((void *)(buffer->data.data() + buffer->readIndex), size);
    buffer->readIndex += size;
    return result;
}

void TCPConnection::shutdown() {
    if (::shutdown(fd(), SHUT_WR) < 0) {
        yolanda_msgx("TCPConnection::shutdown failed, socket == %d", fd());
    }
}

} // namespace simpleweb

