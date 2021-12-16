#include "simpleweb/socket.h"
#include "simpleweb/error.h"
#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>


namespace simpleweb {


int Socket::create_tcpv4_server(int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_exit("open socket failed");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        error_exit("bind failed");

    if (listen(sock, 1024) != 0)
        error_exit("listen failed");

    return sock;
}


void Socket::set_blocking(int fd, bool flag)
{
    int arg = fcntl(fd, F_GETFL);
    long flags = arg & ~O_NONBLOCK;
    if (!flag)
        flags |= O_NONBLOCK;
    (void)fcntl(fd, F_SETFL, flags);
}


} // namespace simpleweb
