#include <sys/epoll.h>
#include <unistd.h>
#include "simpleweb/event_dispatcher.h"
#include "simpleweb/event_loop.h"


namespace simpleweb {


int EventDispatcher::add(Channel *channel) 
{
    int fd = channel->fd();
    struct epoll_event event;
    event.data.ptr = channel;
    event.events = channel->events();
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
        perror("epoll_ctl add  fd failed");
        return -1;
    }
    return 0;
}

int EventDispatcher::del(Channel *channel) 
{
    if (epoll_ctl(efd, EPOLL_CTL_DEL, channel->fd(), nullptr) == -1) {
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
    if (epoll_ctl(efd, EPOLL_CTL_MOD, channel->fd(), &event) == -1) {
        perror("epoll_ctl modify fd failed");
        return -1;
    }

    return 0;
}

int EventDispatcher::dispatch(struct timeval *timeval) 
{
    (void)(timeval);
    int n = epoll_wait(efd, events.data(), events.size(), -1);
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
        auto channel = (Channel *)events[i].data.ptr;
        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
            fprintf(stderr, "epoll error\n");
            channel->error();
            continue;
        }

        if (events[i].events & EPOLLIN)
            channel->read();

        if (events[i].events & EPOLLOUT) 
            channel->write();
    }

    return 0;
}

EventDispatcher::EventDispatcher() 
    : efd(epoll_create1(0))
    , events(128)
{
}

EventDispatcher::~EventDispatcher() 
{
    close(efd);
}


} // namespace simpleweb
