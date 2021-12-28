#ifndef SIMPLEWEB_TCP_CONNECTION
#define SIMPLEWEB_TCP_CONNECTION

#include "simpleweb/event_loop.h"
#include "simpleweb/channel.h"
#include "simpleweb/buffer.h"

namespace simpleweb {


class TCPConnection : public Channel
{
public:
    TCPConnection(int fd, EventLoop *eventLoop);

    ~TCPConnection();
    int send_data(void *data, size_t size);
    int send_buffer(Buffer *buffer);
    void shutdown();

    void handle_connection_closed();

    std::string name;
    Buffer input_buffer;   //接收缓冲区
    Buffer output_buffer;  //发送缓冲区

protected:
    void read() override;
    void write() override;
    void error() override;
    void close() override;

    virtual void on_connection_completed() {}
    virtual void on_connection_closed() {}
    virtual void on_message(Buffer *) {}
    virtual void on_write_completed() {}
};

} // namespace simpleweb

#endif
