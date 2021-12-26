#ifndef SIMPLEWEB_EVENT_LOOP_THREAD_H
#define SIMPLEWEB_EVENT_LOOP_THREAD_H

#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include "simpleweb/event_loop.h"


namespace simpleweb {
    

class EventLoopThread 
{
public:
    //初始化已经分配内存的event_loop_thread
    EventLoopThread(std::string thread_name);
    ~EventLoopThread();

    //由主线程调用，初始化一个子线程，并且让子线程开始运行event_loop
    EventLoop* start();

    friend class EventLoopThreadPool;
protected:
    void run();
private:
    std::unique_ptr<EventLoop> loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::string thread_name_;
};


} // namespace simpleweb


#endif
