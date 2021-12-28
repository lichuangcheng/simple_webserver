#include <assert.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "simpleweb/acceptor.h"
#include "simpleweb/log.h"


namespace simpleweb {
    

void set_blocking(int fd, bool flag)
{
    int arg = fcntl(fd, F_GETFL);
    long flags = arg & ~O_NONBLOCK;
    if (!flag)
        flags |= O_NONBLOCK;
    (void)fcntl(fd, F_SETFL, flags);
}

void make_nonblocking(int fd) {
    set_blocking(fd, false);
}

Acceptor::Acceptor(int port) 
{
    port_ = port;
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    // make_nonblocking(listen_fd);
    non_blocking(true);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listen_fd_, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        error(1, errno, "bind failed ");
    }

    int rt2 = listen(listen_fd_, SOMAXCONN);
    if (rt2 < 0) {
        error(1, errno, "listen failed ");
    }

    signal(SIGPIPE, SIG_IGN);
}


int Acceptor::accept(std::string &ip, uint16_t &port)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int connected_fd = ::accept(listen_fd_, (struct sockaddr *) &client_addr, &client_len);
    if (connected_fd != -1) {
        ip   = inet_ntoa(client_addr.sin_addr);
        port = ntohs(client_addr.sin_port);
    }
    return connected_fd;
}

int Acceptor::socket_fd() 
{
    return listen_fd_;
}

void Acceptor::close() 
{
    if (listen_fd_ != -1) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
}

bool Acceptor::is_open() 
{
    return listen_fd_ != -1;
}

bool Acceptor::non_blocking() const
{
    // FIXME: 
    // if (listen_fd == -1)
        
    int arg = fcntl(listen_fd_, F_GETFL);
    return arg & O_NONBLOCK;
}

void Acceptor::non_blocking(bool mode) 
{
    int arg = fcntl(listen_fd_, F_GETFL);
    long flags = arg & ~O_NONBLOCK;
    if (mode)
        flags |= O_NONBLOCK;
    (void)fcntl(listen_fd_, F_SETFL, flags);
}


Acceptor::~Acceptor()
{
    close();
}


} // namespace simpleweb
