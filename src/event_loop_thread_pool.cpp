#include <assert.h>
#include "simpleweb/event_loop_thread_pool.h"


namespace simpleweb {


EventLoopThreadPool::EventLoopThreadPool(EventLoop *mainLoop, int threadNumber)
    : mainLoop(mainLoop)
    , thread_number(threadNumber)
{
    eventLoopThreads.reserve(threadNumber);
}

//一定是main thread发起
void EventLoopThreadPool::start()
{
    assert(!started);
    mainLoop->assert_in_loop_thread();

    started = 1;
    if (thread_number <= 0) 
        return;

    for (int i = 0; i < thread_number; i++)
        eventLoopThreads.emplace_back(new EventLoopThread("Thread-" + std::to_string(i)))->start();
}

//一定是main thread中选择
EventLoop* EventLoopThreadPool::get_loop()
{
    assert(started);
    mainLoop->assert_in_loop_thread();

    //优先选择当前主线程
    auto selected = mainLoop;
    //从线程池中按照顺序挑选出一个线程
    if (thread_number > 0)
    {
        selected = eventLoopThreads[position]->loop_.get();
        if (++position >= thread_number)
        {
            position = 0;
        }
    }

    return selected;
}

} // namespace simpleweb

