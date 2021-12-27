#include "simpleweb/event_loop.h"
#include <simpleweb/tcp_server.h>
#include <simpleweb/tcp_connection_factory.h>

using namespace simpleweb;


class ClientConnection : public TCPConnection
{
public:
    ClientConnection(int fd, EventLoop *eventLoop) : TCPConnection(fd, eventLoop)
    {
    }

    ~ClientConnection()
    {
    }
protected:
    void on_connection_completed() override
    {
        printf("connection completed: fd = %d\n", fd());
    }

    void on_connection_closed() override
    {
        printf("connection closed. fd = %d\n", fd());
    }

    void on_message(Buffer *input) override
    {
        printf("get message from tcp connection %s\n", name.c_str());
        printf("%s", input->data.c_str());

        Buffer output;
        size_t size = input->readable();
        for (size_t i = 0; i < size; i++) {
            output.append(std::toupper(input->read_char()));
        }
        send_buffer(&output);
    }

    void on_write_completed() override
    {
        printf("write completed: %d\n", fd());
    }
};

typedef TCPConnectionFactoryImpl<ClientConnection> TCPFactory;

int main(int argc, char const *argv[])
{
    int port = 8080;
    if (argc >= 2) 
        port = std::stoi(argv[1]);

    printf("listen on 0.0.0.0:%d\n", port);
    
    EventLoop ev_loop;
    TCPServer svr(std::make_shared<TCPFactory>(), &ev_loop, port, 0);

    svr.start();
    ev_loop.run();
    return 0;
}
