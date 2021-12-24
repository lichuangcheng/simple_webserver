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
    EventLoop();
    EventLoop(const std::string &thread_name);

    int run();
    void stop();

    // 唤醒
    void wakeup();

    // 增加通道事件
    int add_channel(Channel *channel);

    int remove_channel(Channel *channel);

    int update_channel(Channel *channel);

    bool has_channel(Channel *channel);

    // dispather派发完事件之后，调用该方法通知EventLoop执行对应事件的相关callback方法
    // res: EVENT_READ | EVENT_READ等
    int channel_event_activate(int revents);

    void assert_in_same_thread();

    int is_in_same_thread();

    ~EventLoop();
protected:
    int handle_pending_add(Channel *channel);

    int handle_pending_remove(Channel *channel);

    int handle_pending_update(Channel *channel);
private:
    int handle_pending_channel();
    int do_channel_event(Channel *channel, int type);
    void handle_wakeup();

    int quit;
    const std::string thread_name;
    std::thread::id owner_thread_id;
    int is_handle_pending;

    std::unique_ptr<EventDispatcher> eventDispatcher;

    typedef std::pair<Channel*, int> ChannelElement;
    std::vector<ChannelElement> pending_list;

    std::mutex mutex;
    int socketPair[2];
};



} // namespace simpleweb

#endif
