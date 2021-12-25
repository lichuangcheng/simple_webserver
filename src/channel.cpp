#include <simpleweb/channel.h>
#include <simpleweb/event_loop.h>
#include <sys/epoll.h>
#include <unistd.h>


namespace simpleweb {


int Channel::is_enabled_write() const 
{
    return events_ & EPOLLOUT;
}


void Channel::enable_write()
{
    events_ = events_ | EPOLLOUT;
    loop_->update_channel(shared_from_this());
}


void Channel::disable_write()
{
    events_ = events_ & ~EPOLLOUT;
    loop_->update_channel(shared_from_this());
}


void Channel::close_fd()
{
    if (fd_ != -1)
        ::close(fd_);
}


Channel::~Channel()
{
    printf("[info] Channel::~Channel() fd == %d\r\n", fd_);
}


} // namespace simpleweb
