#if !defined(SIMPLEWEB_TCP_CONNECTION_FACTORY)
#define SIMPLEWEB_TCP_CONNECTION_FACTORY


#include "simpleweb/tcp_connection.h"


namespace simpleweb {


class EventLoop;


class TCPConnectionFactory
{
public:
    using Ptr = std::shared_ptr<TCPConnectionFactory>;
    virtual ~TCPConnectionFactory() = default;

    virtual std::shared_ptr<TCPConnection> create_connection(int conn_fd, EventLoop *loop) = 0;
protected:
    TCPConnectionFactory() = default;

private:
    TCPConnectionFactory(const TCPConnectionFactory &) = delete;
    TCPConnectionFactory& operator = (const TCPConnectionFactory &) = delete;
};


template <typename C>
class TCPConnectionFactoryImpl : public TCPConnectionFactory
{
public:
    TCPConnectionFactoryImpl() = default;
    ~TCPConnectionFactoryImpl() = default;

    std::shared_ptr<TCPConnection> create_connection(int conn_fd, EventLoop *loop) override
    {
        return std::make_shared<C>(conn_fd, loop);
    }
};


} // namespace simpleweb


#endif // SIMPLEWEB_TCP_CONNECTION_FACTORY
