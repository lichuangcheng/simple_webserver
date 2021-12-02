#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <array>

#include "simpleweb/ini_config.h"

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

void accept_request(int sock, const std::string &index_html)
{
    std::ostringstream ss;
    ss << "HTTP/1.0 200 OK\r\n";
    ss << "Content-Type: text/html;charset=utf-8\r\n";
    ss << "Content-Length: " << index_html.size() << "\r\n";
    ss << "\r\n";
    ss << index_html;

    std::array<char, 4096> buf;
    int n = 0;
    while((n = recv(sock, buf.data(), buf.size() - 1, 0)) > 0)
    {
        char method[255];
        int i = 0;
        while(!isspace(buf[i]) && i < n && i < sizeof(method) - 1)
        {
            method[i] = buf[i];
            i++;
        }
        method[i] = '\0';
        if (!strcasecmp(method, "GET"))
        {
            auto rep = ss.str();
            send(sock, rep.data(), rep.size(), 0);
        }
    }

    printf("connection close!\n");
    close(sock);
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

    simpleweb::IniConfig conf(argv[1]);
    uint16_t port        = conf.get_int32("server.port");
    std::string root_dir = conf.get_string("server.root");

    int sock = create_tcp_server(port);
    printf("root: %s \n", root_dir.c_str());
    printf("listen on 0.0.0.0:%d\n", port);
    
    const auto index_html = readfile((root_dir + "/index.html").c_str());

    int client_sock = -1;
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    while(1)
    {
        client_sock = accept(sock, (struct sockaddr *)&client_name, &client_name_len);
        if (client_sock == -1) 
            error_exit("accept failed");

        printf("accept connection: %s\n", inet_ntoa(client_name.sin_addr));
        std::thread(accept_request, client_sock, std::ref(index_html)).detach();
    }

    close(sock);
    return 0;
}
