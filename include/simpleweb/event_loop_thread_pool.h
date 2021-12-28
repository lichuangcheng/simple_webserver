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
    EventLoopThreadPool(EventLoop *main_loop, int thread_num);
    ~EventLoopThreadPool() = default;

    void start();

    EventLoop *get_loop();
private:
    EventLoop *main_loop_ {nullptr};
    //是否已经启动
    int started_ {0};
    //线程数目
    int thread_number_;
    //数组指针，指向创建的event_loop_thread数组
    std::vector<std::unique_ptr<EventLoopThread>> ev_loop_threads_;
    //表示在数组里的位置，用来决定选择哪个event_loop_thread服务
    int pos_ {0};
};

} // namespace simpelweb

#endif
