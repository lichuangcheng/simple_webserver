#include <vector>
#include <fstream>
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
#include <simpleweb/tcp_connection_factory.h>

using std::cout;
using std::endl;

namespace fs = std::filesystem;
using namespace simpleweb;
using namespace std::placeholders;

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

void bad_request(HttpResponse &res)
{
    res.status = 400;
    res.set_content("400 Bad request", "text/html;charset=utf-8");
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

class ClientConnection : public TCPConnection
{
public:
    ClientConnection(std::string root, int fd, EventLoop *eventLoop) : TCPConnection(fd, eventLoop), root_(std::move(root))
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

        HttpRequest req;
        HttpResponse res;
        res.version = "HTTP/1.0";
        if (req.parse(std::string_view(input->read_begin(), input->readable()))) 
        {
            if (iequals(req.method, "GET"))
                request_file(root_, req, res);
            else
                unimplemented(res);
        }
        else bad_request(res);
            
        res.reason = HttpResponse::status_message(res.status);
        res.headers["Server"] = "simpleweb/0.1.0";
        auto s = res.to_string();
        
        send_data(s.data(), s.size());
    }

    void on_write_completed() override
    {
        printf("write completed: %d\n", fd());
    }

private:
    std::string root_;
};


class ClientTCPConnectionFactory : public TCPConnectionFactory
{
public:
    ClientTCPConnectionFactory(std::string root) : root_(root)
    {
    }

    ~ClientTCPConnectionFactory()
    {
    }

    std::shared_ptr<TCPConnection> create_connection(int conn_fd, EventLoop *loop) override
    {
        return std::make_shared<ClientConnection>(root_, conn_fd, loop);
    }
private:
    std::string root_;
};

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

    printf("root: %s \n", root_dir.c_str());
    printf("listen on 0.0.0.0:%d\n", port);
    
    EventLoop ev_loop;
    TCPServer svr(std::make_shared<ClientTCPConnectionFactory>(root_dir), &ev_loop, port, 0);

    svr.start();
    ev_loop.run();
    return 0;
}
