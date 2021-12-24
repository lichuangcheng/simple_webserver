#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <string_view>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <optional>

#include "simpleweb/ini_config.h"
#include "simpleweb/string_utils.h"
#include "simpleweb/http_request.h"
#include "simpleweb/http_response.h"
#include "simpleweb/error.h"
#include "simpleweb/socket.h"
#include "simpleweb/event_loop.h"

using std::cout;
using std::endl;

namespace fs = std::filesystem;
using namespace simpleweb;

std::optional<std::string> file_media_type(const std::string &ext)
{
    static const std::unordered_map<std::string, std::string> media_type = {
        {".css",  "text/css"},
        {".txt",  "text/plain;charset=utf-8"},
        {".html",  "text/html;charset=utf-8"},

        {".gif",  "image/gif"},
        {".png",  "image/png"},
        {".ico",  "image/x-icon"},
        {".jpeg",  "image/jpeg"},

        {".pdf", "application/pdf"},
        {".js", "application/javascript"},
        {".json", "application/json"},

        {".xml", "application/xml"},
        {".gz", "application/gzip"},
        {".zip", "application/zip"},
    };
    auto it = media_type.find(ext);
    if (it != media_type.end()) return it->second;
    return std::nullopt;
}

std::string readfile(const std::string &file)
{
    std::string out;
    std::ifstream fs(file, std::ios_base::binary);
    fs.seekg(0, std::ios_base::end);
    auto size = fs.tellg();
    fs.seekg(0);
    out.resize(static_cast<size_t>(size));
    fs.read(&out[0], static_cast<std::streamsize>(size));
    return out;
}

void not_found(HttpResponse &res)
{
    static const std::string text = "404 NOT FOUND";
    res.status = 404;
    res.set_content(text, "text/html;charset=utf-8");
}

void unimplemented(HttpResponse &res)
{
    res.status = 501;
    res.set_content("Method Not Implemented", "text/html;charset=utf-8");
}

void request_file(const std::string &root, const HttpRequest &req, HttpResponse &res)
{
    printf("request file: %s \n", req.path.c_str());
    auto full_path = root + req.path;
    if (req.path == "/") full_path += "index.html";
    cout << "full path: " << full_path << endl;

    if (!fs::exists(full_path) || !fs::is_regular_file(full_path))
        return not_found(res);

    auto file = readfile(full_path);
    auto type = file_media_type(fs::path(full_path).extension());
    res.set_content(std::move(file), type.value_or("text/plain;charset=utf-8"));
    res.status = 200;
    return;
}

int request_process(int sock, const std::string &root)
{
    std::array<char, 4096> buf;
    int n = 0;
    if((n = recv(sock, buf.data(), buf.size() - 1, 0)) > 0)
    {
        HttpRequest req;
        HttpResponse res;
        res.version = "HTTP/1.0";
        req.parse(std::string_view(buf.data(), n));
        if (iequals(req.method, "GET"))
            request_file(root, req, res);
        else 
            unimplemented(res);
        res.reason = HttpResponse::status_message(res.status);
        res.headers["Server"] = "simpleweb/0.1.0";
        auto s = res.to_string();
        send(sock, s.data(), s.size(), 0);
    }
    return n;
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>. \n", argv[0]);
        return 1;
    }

    IniConfig conf(argv[1]);
    uint16_t port        = conf.get_int32("server.port");
    std::string root_dir = conf.get_string("server.root");

    int sock = simpleweb::Socket::create_tcpv4_server(port);
    printf("root: %s \n", root_dir.c_str());
    printf("listen on 0.0.0.0:%d\n", port);
    
    auto epfd = epoll_create(1);
    if (epfd < 0) error_exit("epoll_create");

    epoll_event svr_ev;
    svr_ev.events = EPOLLIN;
    svr_ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &svr_ev);

    EventLoop ev_loop;
    auto accept_chan = std::make_shared<ChannelAdaptor>(sock, EPOLLIN, &ev_loop);

    accept_chan->read([&] 
    {
        struct sockaddr_in client_name;
        socklen_t client_name_len = sizeof(client_name);
        int connfd = accept(accept_chan->fd(), (struct sockaddr *)&client_name, &client_name_len);
        if (connfd == -1) 
            error_exit("accept failed");

        printf("accept connection: %s:%d\n", inet_ntoa(client_name.sin_addr), ntohs(client_name.sin_port));

        Socket::set_noblock(connfd);
        auto conn_chan = ChannelAdaptor::create(connfd, EPOLLIN, &ev_loop);
        conn_chan->read([conn_chan, root_dir, &ev_loop] 
        {
            printf("connect read!\r\n");
            int n = request_process(conn_chan->fd(), root_dir);
            if (n == 0)
            {
                printf("connection close!\n");
                ev_loop.remove_channel(conn_chan);
                conn_chan->close_fd();
                delete conn_chan;
            }
            if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EINTR) 
                    return;
                else 
                {
                    perror("recv");
                    ev_loop.remove_channel(conn_chan);
                    conn_chan->close_fd();
                    delete conn_chan;
                }
            }
        });
        ev_loop.add_channel(conn_chan);
    });
    ev_loop.add_channel(accept_chan.get());

    ev_loop.run();
    close(sock);
    return 0;
}
