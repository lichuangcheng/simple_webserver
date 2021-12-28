#ifndef SIMPLEWEB_EVENT_LOOP_H
#define SIMPLEWEB_EVENT_LOOP_H

#include "simpleweb/channel.h"
#include "simpleweb/event_dispatcher.h"
#include <memory>
#include <mutex>
#include <thread>
#include <map>


namespace simpleweb {
    

class EventLoop 
{
public:
    // 构造
    EventLoop();
    EventLoop(const std::string &thread_name);

    // 进入事件循环
    int run();

    // 停止事件循环
    void stop();

    // 唤醒
    void wakeup();

    // 增加通道事件
    int add_channel(ChannelPtr channel);

    // 删除通道
    int remove_channel(ChannelPtr channel);

    // 更新通道信息
    int update_channel(ChannelPtr channel);

    // 断言调用线程是否与loop线程一致
    void assert_in_loop_thread();
    int is_in_loop_thread();

    // 析构
    ~EventLoop();
protected:
    int handle_pending_add(ChannelPtr channel);
    int handle_pending_remove(ChannelPtr channel);
    int handle_pending_update(ChannelPtr channel);

private:
    int handle_pending_channel();
    int do_channel_event(ChannelPtr channel, int type);
    void handle_wakeup();

    typedef std::pair<ChannelPtr, int> ChannelElement;

    int quit_;
    const std::string thread_name;
    std::thread::id owner_thread_id;
    int is_handle_pending_;

    std::unique_ptr<EventDispatcher> dispatcher_;
    std::vector<ChannelElement> pending_list_;
    std::map<int, std::shared_ptr<Channel>> channels_;

    std::mutex mutex_;
    int socketpair_[2];
};



} // namespace simpleweb

#endif
