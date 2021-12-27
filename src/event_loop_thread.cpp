#include <assert.h>
#include "simpleweb/event_loop_thread.h"
#include "simpleweb/event_loop.h"
#include "simpleweb/log.h"

namespace simpleweb 
{
    
//初始化已经分配内存的event_loop_thread
EventLoopThread::EventLoopThread(std::string thread_name) 
    : thread_name_(std::move(thread_name))
{
}

EventLoopThread::~EventLoopThread()
{
    if (loop_) {
        loop_->stop();
        if (thread_.joinable())
            thread_.join();
    }
}

//由主线程调用，初始化一个子线程，并且让子线程开始运行event_loop
EventLoop *EventLoopThread::start()
{
    thread_ = std::thread(&EventLoopThread::run, this);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            return this->loop_ != nullptr;
        });
    }
    
    yolanda_msgx("event loop thread started, %s", thread_name_.c_str());
    return loop_.get();
}

void EventLoopThread::run()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 初始化化event loop，之后通知主线程
        loop_ = std::make_unique<EventLoop>(thread_name_);
        yolanda_msgx("event loop thread init and signal, %s", thread_name_.c_str());
        cond_.notify_one();
    }

    // 子线程event loop run
    loop_->run();
}

} // namespace simpleweb
