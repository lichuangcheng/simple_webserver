#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

struct client_socket_t {
    int fd;
    struct sockaddr_in addr;
};

void error_exit(const char *s)
{
    perror(s);
    exit(1);
}

int sendhtml(int sock, const char *file)
{
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
        error_exit("open file failed");

    struct stat file_st;
    stat(file, &file_st);
    size_t file_size = file_st.st_size;

    char *html = calloc(1, file_size + 1);
    fread(html, 1, file_size, fp);
    send(sock, html, file_size, 0);

    fclose(fp);
    return 0;
}

void *accept_request(void *arg)
{
    pthread_detach(pthread_self());
    static char rep[] = "HTTP/1.0 200 OK\r\n"
                        "Content-Type: text/html;charset=utf-8\r\n"
                        "\r\n";
    
    struct client_socket_t *client = (struct client_socket_t *)arg;
    int sock = client->fd;
    printf("accept connection: %s\n", inet_ntoa(client->addr.sin_addr));

    char buf[2048] = {0};
    int n = recv(client->fd, buf, sizeof(buf), 0);

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
        send(sock, rep, strlen(rep), 0);
        sendhtml(client->fd, "index.html");
    }

    close(client->fd);
    free(client);
    return 0;
}

int main(int argc, char const *argv[])
{
    uint16_t port = 8080;
    if (argc == 2)
        port = atoi(argv[1]);

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

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        error_exit("bind failed");

    if (listen(sock, 20) != 0)
        error_exit("listen failed");

    printf("listen on 0.0.0.0:%d\n", port);
    
    int client_sock = -1;
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    while(1)
    {
        client_sock = accept(sock, (struct sockaddr *)&client_name, &client_name_len);
        if (client_sock == -1) 
            error_exit("accept failed");
        
        struct client_socket_t *client = calloc(1, sizeof(struct client_socket_t));
        client->fd = client_sock;
        memcpy(&client->addr, &client_name, sizeof(client_name));

        pthread_t tid;
        if (pthread_create(&tid, NULL, accept_request, (void *)client) != 0)
            perror("pthread_create");
    }

    close(sock);
    return 0;
}
