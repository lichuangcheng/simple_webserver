#include "simpleweb/event_loop.h"
#include <simpleweb/tcp_server.h>

using namespace simpleweb;

//连接建立之后的callback
int onConnectionCompleted(TCPConnection *conn) 
{
    printf("connection completed: fd = %d\n", conn->fd());
    return 0;
}

//数据读到buffer之后的callback
int onMessage(buffer *input, TCPConnection *conn) 
{
    printf("get message from tcp connection %s\n", conn->name.c_str());
    printf("%s", input->data.c_str());

    buffer output;
    size_t size = input->readable();
    for (size_t i = 0; i < size; i++) {
        output.append(std::toupper(input->read_char()));
    }
    conn->send_buffer(&output);
    return 0;
}

//数据通过buffer写完之后的callback
int onWriteCompleted(TCPConnection *conn) 
{
    printf("write completed: %d\n", conn->fd());
    return 0;
}

//连接关闭之后的callback
int onConnectionClosed(TCPConnection *conn) 
{
    printf("connection closed. fd = %d\n", conn->fd());
    return 0;
}

int main(int argc, char const *argv[])
{
    int port = 8080;
    if (argc >= 2) 
        port = std::stoi(argv[1]);

    printf("listen on 0.0.0.0:%d\n", port);
    
    EventLoop ev_loop;
    TCPServer svr(&ev_loop, port, 0);

    svr.on_connection_completed(onConnectionCompleted)
       .on_message(onMessage)
       .on_write_completed(onWriteCompleted)
       .on_connection_close(onConnectionClosed);

    svr.start();
    ev_loop.run();
    return 0;
}
