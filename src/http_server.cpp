#include "simpleweb/http_server.h"
#include "simpleweb/socket.h"
#include "simpleweb/error.h"
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>


namespace simpleweb {


template <typename F>
int handle_EINTR(F f)
{
    while(1)
    {
        int n = f();
        if (n < 0 && errno == EINTR) continue;
        return n;
    }
}


inline std::string readfile(const std::string &file)
{
    printf("request file: %s \n", file.c_str());
    std::ifstream fin(file);
    if (!fin)
        return {};

    std::ostringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}


HttpServer::HttpServer() 
    : tp_(2)
{
}


HttpServer::~HttpServer()
{
    if (sock_ != -1)
        close(sock_);
    tp_.shutdown();
}


bool HttpServer::listen(int port) 
{
    sock_ = Socket::create_tcpv4_server(port);
    // Socket::set_blocking(sock_, false);
    
    auto epfd = epoll_create(1);
    if (epfd < 0) error_exit("epoll_create");

    epoll_event svr_ev;
    svr_ev.events = EPOLLIN;
    svr_ev.data.fd = sock_;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock_, &svr_ev);
    
    while(1)
    {
        std::array<epoll_event, 1024> events;
        int n = epoll_wait(epfd, events.data(), events.size(), 1000);
        if (n == 0)
        {
            printf("epoll_wait timeout!");
            continue;
        }
        if (n < 0)
        {
            if (errno == EINTR) continue;
            else break;
        }
        for (int i = 0; i < n; i++)
        {
            if (events[i].data.fd == sock_ && events[i].events & EPOLLIN)
            {
                struct sockaddr_in client_name;
                socklen_t client_name_len = sizeof(client_name);
                int connfd = accept(events[i].data.fd, (struct sockaddr *)&client_name, &client_name_len);
                if (connfd == -1) 
                    error_exit("accept failed");

                printf("accept connection: %s\n", inet_ntoa(client_name.sin_addr));
                // close(connfd);

                epoll_event ev;
                ev.events = EPOLLIN | EPOLLERR;
                ev.data.fd = connfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else 
            {
                if (events[i].events & EPOLLERR)
                {
                    int err = Socket::socket_err(events[i].data.fd);
                    if (err != 0) 
                        printf("socket error: %s\r\n", strerror(err));
                    
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                }
                else if (events[i].events & EPOLLIN)
                {

                }
            }
        }
    }
    return sock_ != -1;
}


} // namespace simpleweb
