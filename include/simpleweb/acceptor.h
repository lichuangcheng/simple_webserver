#ifndef SIMPLEWEB_ACCEPTOR_H
#define SIMPLEWEB_ACCEPTOR_H


#include <string>


namespace simpleweb {


class Acceptor 
{
public:
    Acceptor(int port);
    ~Acceptor();

    int accept(std::string &ip, uint16_t &port);
    int socket_fd();
    void close();
    bool is_open();
    bool non_blocking() const;
    void non_blocking(bool mode);
private:
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator = (const Acceptor&) = delete;
    
    int listen_port;
    int listen_fd {-1};
};


void set_blocking(int fd, bool flag);
void make_nonblocking(int fd);


} // namespace simpleweb


#endif
