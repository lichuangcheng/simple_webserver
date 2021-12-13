#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>
#include <set>
#include <string_view>
#include <assert.h>
#include <algorithm>
#include <iostream>

#include "simpleweb/ini_config.h"
#include "simpleweb/string_utils.h"
#include "simpleweb/http_request.h"

using simpleweb::trim;
using simpleweb::iequals;
using simpleweb::IniConfig;
using simpleweb::HttpRequest;

using std::cout;
using std::endl;

void error_exit(const char *s)
{
    perror(s);
    exit(1);
}

std::string readfile(const char *file)
{
    std::ifstream fin(file);
    if (!fin)
        error_exit("open file failed");

    std::ostringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

bool parse_request_header(std::string_view d, HttpRequest &req)
{
    static const std::set<std::string> methods {
      "GET",     "HEAD",    "POST",  "PUT",   "DELETE",
      "CONNECT", "OPTIONS", "TRACE", "PATCH", "PRI"};

    std::string method;
    std::string url;
    std::string version;

    size_t i = 0;
    auto n = d.size();
    while(!std::isspace(d[i]) && i < n) { method += d[i]; i++; }
    if (!methods.contains(simpleweb::to_upper(method))) return false;

    while(std::isspace(d[i])) i++;
    while(!isspace(d[i]) && i < n) { url += d[i]; i++; }

    while(std::isspace(d[i])) i++;
    while(!isspace(d[i]) && i < n) { version += d[i]; i++; }

    req.method = std::move(method);
    req.path = std::move(url);
    req.version = std::move(version);
    return true;
}

bool parse_request(const char *d, size_t n, HttpRequest &req)
{
    std::string_view str(d, n);
    const auto npos = str.npos;
    auto crlf = str.find("\r\n");
    if (crlf == npos) return false;

    if (!parse_request_header(str.substr(0, crlf), req)) return false;
    
    auto begin = crlf + 2;
    while(begin < str.size() && (crlf = str.find("\r\n", begin)) != str.npos && crlf != 0)
    {
        std::string_view line = str.substr(begin, crlf - begin);
        auto sep = line.find(':');
        if (sep != line.npos)
        {
            req.headers.emplace(line.substr(0, sep), trim(line.substr(sep + 1)));
        }
        begin = crlf + 2;
    }

    return true;
}

int request_process(int sock, const std::string &index_html)
{
    std::ostringstream ss;
    ss << "HTTP/1.0 200 OK\r\n";
    ss << "Content-Type: text/html;charset=utf-8\r\n";
    ss << "Content-Length: " << index_html.size() << "\r\n";
    ss << "\r\n";
    ss << index_html;
    auto rep = ss.str();

    std::array<char, 4096> buf;
    int n = 0;
    if((n = recv(sock, buf.data(), buf.size() - 1, 0)) > 0)
    {
        HttpRequest req;
        parse_request(buf.data(), n, req);
        cout << req << endl;

        if (iequals(req.method, "GET"))
            send(sock, rep.data(), rep.size(), 0);
    }
    return n;
}

int create_tcp_server(uint16_t port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) 
        error_exit("open socket failed");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) 
        error_exit("setsockopt failed");
    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on))) < 0) 
        error_exit("setsockopt failed");

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        error_exit("bind failed");

    if (listen(sock, 20) != 0)
        error_exit("listen failed");

    return sock;
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

    int sock = create_tcp_server(port);
    printf("root: %s \n", root_dir.c_str());
    printf("listen on 0.0.0.0:%d\n", port);
    
    const auto index_html = readfile((root_dir + "/index.html").c_str());

    int client_sock = -1;
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    fd_set readfds;
    FD_ZERO(&readfds);

    std::vector<int> client_fds;
    client_fds.reserve(1024);

    while(1)
    {
        // struct timeval tv = {1, 0};
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        
        int max_fd = sock;
        for (int fd : client_fds)
        {
            if (fd != -1)
            {
                FD_SET(fd, &readfds);
                max_fd = std::max(max_fd, fd);
            }
        }

        int retval = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (retval == 0)
        {
            printf("selcet timeout.\n");
            continue;
        }
        if (retval < 0) error_exit("select");

        if (FD_ISSET(sock, &readfds))
        {
            client_sock = accept(sock, (struct sockaddr *)&client_name, &client_name_len);
            if (client_sock == -1) 
                error_exit("accept failed");

            printf("accept connection: %s\n", inet_ntoa(client_name.sin_addr));
            client_fds.push_back(client_sock);
            if (client_fds.size() > 64)
            {
                client_fds.erase(std::remove_if(client_fds.begin(), client_fds.end(), [](int fd) { return fd == -1; }), 
                                client_fds.end());
            }
        }
        else 
        {
            for (size_t i = 0; i < client_fds.size(); ++i)
            {
                int fd = client_fds[i];
                if (fd != -1 && FD_ISSET(fd, &readfds))
                {
                    int n = request_process(fd, index_html);
                    if (n == 0)
                    {
                        printf("connection close!\n");
                        close(fd);
                        client_fds[i] = -1;
                    }
                    if (n < 0 && (errno != EINTR || errno != EWOULDBLOCK))
                    {
                        perror("read");
                        close(fd);
                        client_fds[i] = -1;
                    }
                }
            }
        }
    }

    close(sock);
    return 0;
}
