#include <assert.h>
#include "simpleweb/event_loop_thread_pool.h"


namespace simpleweb {


EventLoopThreadPool::EventLoopThreadPool(EventLoop *main_loop, int threadNumber)
    : main_loop_(main_loop)
    , thread_number_(threadNumber)
{
    ev_loop_threads_.reserve(threadNumber);
}

//一定是main thread发起
void EventLoopThreadPool::start()
{
    assert(!started_);
    main_loop_->assert_in_loop_thread();

    started_ = 1;
    if (thread_number_ <= 0) 
        return;

    for (int i = 0; i < thread_number_; i++)
        ev_loop_threads_.emplace_back(new EventLoopThread("Thread-" + std::to_string(i)))->start();
}

//一定是main thread中选择
EventLoop* EventLoopThreadPool::get_loop()
{
    assert(started_);
    main_loop_->assert_in_loop_thread();

    //优先选择当前主线程
    auto selected = main_loop_;
    //从线程池中按照顺序挑选出一个线程
    if (thread_number_ > 0)
    {
        selected = ev_loop_threads_[pos_]->loop_.get();
        if (++pos_ >= thread_number_)
        {
            pos_ = 0;
        }
    }

    return selected;
}

} // namespace simpleweb

