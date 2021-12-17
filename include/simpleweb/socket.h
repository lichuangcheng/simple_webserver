#if !defined(SIMPLEWEB_SOCKET_INCLUDED)
#define SIMPLEWEB_SOCKET_INCLUDED


namespace simpleweb {


class Socket
{
public:
    static int create_tcpv4_server(int port);
    static void set_blocking(int fd, bool flag);
    static void set_noblock(int fd);
    static int socket_err(int sock);
};


} // namespace simpleweb


#endif // SIMPLEWEB_SOCKET_INCLUDED

