#ifndef SIMPLEWEB_EVENT_LOOP_THREAD_POOL_INCLUDED
#define SIMPLEWEB_EVENT_LOOP_THREAD_POOL_INCLUDED

#include "simpleweb/event_loop.h"
#include "simpleweb/event_loop_thread.h"
#include <vector>
#include <memory>

namespace simpleweb {
    
class EventLoopThreadPool 
{
public:
    //创建thread_pool的主线程
    EventLoopThreadPool(EventLoop *mainLoop, int threadNumber);
    ~EventLoopThreadPool() = default;

    void start();

    EventLoop *get_loop();
private:
    EventLoop *mainLoop {nullptr};
    //是否已经启动
    int started {0};
    //线程数目
    int thread_number;
    //数组指针，指向创建的event_loop_thread数组
    std::vector<std::unique_ptr<EventLoopThread>> eventLoopThreads;
    //表示在数组里的位置，用来决定选择哪个event_loop_thread服务
    int position {0};
};

} // namespace simpelweb

#endif
