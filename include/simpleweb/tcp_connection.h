#ifndef SIMPLEWEB_TCP_CONNECTION
#define SIMPLEWEB_TCP_CONNECTION

#include "simpleweb/event_loop.h"
#include "simpleweb/channel.h"
#include "simpleweb/buffer.h"

namespace simpleweb {

class TCPConnection;

using connection_completed_call_back = std::function<int (TCPConnection *)>;
using message_call_back = std::function<int (buffer *, TCPConnection *)>;
using write_completed_call_back = std::function<int (TCPConnection *)>;
using connection_closed_call_back = std::function<int (TCPConnection *)>;

class TCPConnection : public Channel
{
public:
    TCPConnection(int fd, EventLoop *eventLoop, connection_completed_call_back connectionCompletedCallBack,
                   connection_closed_call_back connectionClosedCallBack,
                   message_call_back messageCallBack, write_completed_call_back writeCompletedCallBack);

    ~TCPConnection();
    int send_data(void *data, size_t size);
    int send_buffer(buffer *buffer);
    void shutdown();

    void handle_connection_closed();

    friend class TCPServer;
protected:
    void read() override;
    void write() override;
    void error() override;
    void close() override;

private:
    std::string name;
    buffer input_buffer;   //接收缓冲区
    buffer output_buffer;  //发送缓冲区

    connection_completed_call_back connectionCompletedCallBack;
    connection_closed_call_back connectionClosedCallBack;
    message_call_back messageCallBack;
    write_completed_call_back writeCompletedCallBack; 

    void * data; //for callback use: http_server
    void * request; // for callback use
    void * response; // for callback use
};

} // namespace simpleweb

#endif
