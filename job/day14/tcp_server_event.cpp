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
#include <simpleweb/acceptor.h>
#include <simpleweb/tcp_server.h>

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

//连接建立之后的callback
int onConnectionCompleted(TCPConnection *conn) {
    printf("connection completed: fd = %d\n", conn->chan->fd);
    return 0;
}

//数据读到buffer之后的callback
int onMessage(struct buffer *input, TCPConnection *tcpConnection) {
    printf("get message from tcp connection %s\n", tcpConnection->name.c_str());
    printf("%s", input->data.c_str());

    buffer output;
    int size = input->readable();
    for (int i = 0; i < size; i++) {
        output.append(rot13_char(input->read_char()));
    }
    tcpConnection->send_buffer(&output);
    return 0;
}

//数据通过buffer写完之后的callback
int onWriteCompleted(TCPConnection *tcpConnection) {
    printf("write completed\n");
    return 0;
}

//连接关闭之后的callback
int onConnectionClosed(TCPConnection *tcpConnection) {
    printf("connection closed. fd = %d\n", tcpConnection->fd());
    return 0;
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

    Acceptor acceptor(port);
    printf("root: %s \n", root_dir.c_str());
    printf("listen on 0.0.0.0:%d\n", port);
    
    EventLoop ev_loop;
    TCPServer tcp_svr(&ev_loop, port, 0);

    auto accept_chan = std::make_shared<ChannelAdaptor>(acceptor.socket_fd(), EPOLLIN, &ev_loop);
    accept_chan->read([root_dir, &ev_loop, &acceptor] 
    {
        std::string ip;
        uint16_t port;
        auto connfd = acceptor.accept(ip, port);
        printf("accept connection: %s:%d\n", ip.c_str(), port);

        Socket::set_noblock(connfd);
        auto conn_chan = std::make_shared<ChannelAdaptor>(connfd, EPOLLIN, &ev_loop);
        std::weak_ptr<ChannelAdaptor> conn_weak(conn_chan);
        conn_chan->read([conn_weak, root_dir, &ev_loop] 
        {
            auto conn = conn_weak.lock();
            printf("connect read!\r\n");
            int n = request_process(conn->fd(), root_dir);
            if (n == 0)
            {
                printf("connection close!\n");
                ev_loop.remove_channel(conn);
                conn->close_fd();
            }
            if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EINTR) 
                    return;
                else 
                {
                    perror("recv");
                    ev_loop.remove_channel(conn);
                    conn->close_fd();
                }
            }
        });
        ev_loop.add_channel(conn_chan);
    });
    ev_loop.add_channel(accept_chan);
    ev_loop.run();
    return 0;
}
