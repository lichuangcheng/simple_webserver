#ifndef SIMPLEWEB_EVENT_DISPATCHER_H
#define SIMPLEWEB_EVENT_DISPATCHER_H


#include "simpleweb/channel.h"
#include "sys/epoll.h"
#include <vector>


namespace simpleweb {
    

/** 抽象的event_dispatcher结构体，对应的实现如select,poll,epoll等I/O复用. */
class EventDispatcher 
{
public:
    EventDispatcher();
    ~EventDispatcher();

    /** 通知dispatcher新增一个channel事件*/
    int add(Channel* channel);

    /** 通知dispatcher删除一个channel事件*/
    int del(Channel* channel);

    /** 通知dispatcher更新channel对应的事件*/
    int update(Channel* channel);

    /** 实现事件分发，然后调用event_loop的event_activate方法执行callback*/
    int dispatch(struct timeval *);

private:
    int efd;
    std::vector<epoll_event> events;
};


} // namespace simpleweb


#endif
