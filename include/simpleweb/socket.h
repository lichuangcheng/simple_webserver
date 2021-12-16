#if !defined(SIMPLEWEB_SOCKET_INCLUDED)
#define SIMPLEWEB_SOCKET_INCLUDED


namespace simpleweb {


class Socket
{
public:
    static int create_tcpv4_server(int port);
    static void set_blocking(int fd, bool flag);
};


} // namespace simpleweb


#endif // SIMPLEWEB_SOCKET_INCLUDED

