#include <sys/epoll.h>
#include <unistd.h>
#include "simpleweb/event_dispatcher.h"


namespace simpleweb {


int EventDispatcher::add(Channel *channel) 
{
    int fd = channel->fd();
    struct epoll_event event;
    event.data.ptr = channel;
    event.events = channel->events();
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event) == -1) {
        perror("epoll_ctl add  fd failed");
        return -1;
    }
    return 0;
}

int EventDispatcher::del(Channel *channel) 
{
    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, channel->fd(), nullptr) == -1) {
        perror("epoll_ctl delete fd failed");
        return -1;
    }
    return 0;
}

int EventDispatcher::update(Channel *channel) 
{

    struct epoll_event event;
    event.data.ptr = channel;
    event.events = channel->events();
    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, channel->fd(), &event) == -1) {
        perror("epoll_ctl modify fd failed");
        return -1;
    }

    return 0;
}

int EventDispatcher::dispatch(struct timeval *timeval) 
{
    (void)(timeval);
    int n = epoll_wait(epfd_, events_.data(), events_.size(), -1);
    if (n == 0) {
        printf("epoll_wait timeout!!\n");
        return 0;
    }
    if (n < 0) {
        if (errno == EINTR)
            return 0;
        perror("[erroe] epoll_wait:");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        auto &ev = events_[i];
        auto channel = (Channel *)events_[i].data.ptr;

        if ((ev.events & EPOLLHUP) && !(ev.events & EPOLLIN)) {
            printf("[warn] fd = %d , EventDispatcher::dispatch() EPOLLHUP\n", channel->fd());
            channel->close();
        }

        if ((ev.events & EPOLLERR) /* || (ev.events & EPOLLHUP) */ ) {
            fprintf(stderr, "epoll error\n");
            channel->error();
        }

        if (ev.events & EPOLLIN)
            channel->read();

        if (ev.events & EPOLLOUT) 
            channel->write();
    }

    return 0;
}

EventDispatcher::EventDispatcher() 
    : epfd_(epoll_create1(0))
    , events_(128)
{
}

EventDispatcher::~EventDispatcher() 
{
    ::close(epfd_);
}


} // namespace simpleweb
