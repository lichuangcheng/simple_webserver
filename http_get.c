#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(int argc, char const *argv[])
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("open socket failed");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    // struct hostent *p_host = gethostbyname("baidu.com");
    // memcpy(&(addr.sin_addr.s_addr), p_host->h_addr_list[0], sizeof(addr.sin_addr.s_addr));
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    // bzero(&addr.sin_zero, sizeof(addr.sin_zero));

    int err = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (err != 0) {
        perror("connect failed");
        return -1;
    }
    
    char buf[1024] = "GET / HTTP/1.1\r\n"
                     "Host: www.baidu.com\r\n"
                     "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.45 Safari/537.36 Edg/96.0.1054.29\r\n"
                     "\r\n";

    send(sock, buf, strlen(buf), 0);
    bzero(buf, sizeof(buf));

    int n = recv(sock, buf, 1024, 0);
    printf("recv num: %d\n\n%s", n, buf);

    close(sock);
    return 0;
}
