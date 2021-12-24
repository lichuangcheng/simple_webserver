#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "simpleweb/event_loop.h"
#include "simpleweb/event_dispatcher.h"
#include "simpleweb/channel.h"


namespace simpleweb {


// in the i/o thread
int EventLoop::handle_pending_channel() 
{
    // scope lock
    std::lock_guard<std::mutex> lock(mutex);
    is_handle_pending = 1;

    for (auto [channel, type] : pending_list)
    {
        if (type == 1)
            handle_pending_add(channel);
        else if (type == 2)
            handle_pending_remove(channel);
        else if (type == 3)
            handle_pending_update(channel);
    }
    pending_list.clear();
    is_handle_pending = 0;
    return 0;
}

int EventLoop::do_channel_event(Channel *channel, int type) 
{
    {
        // scope lock
        std::lock_guard<std::mutex> lock(mutex);
        assert(is_handle_pending == 0);
         // 将通道事件入队
        pending_list.push_back(std::make_pair(channel, type));
    }

    if (!is_in_same_thread()) 
        wakeup();
    else 
        handle_pending_channel();

    return 0;
}

int EventLoop::add_channel(Channel *channel) 
{
    if (is_in_same_thread())
        return handle_pending_add(channel);
    return do_channel_event(channel, 1);
}

int EventLoop::remove_channel(Channel *channel) 
{
    if (is_in_same_thread())
        return handle_pending_remove(channel);
    return do_channel_event(channel, 2);
}

int EventLoop::update_channel(Channel *channel) 
{
    if (is_in_same_thread())
        return handle_pending_update(channel);
    return do_channel_event(channel, 3);
}

// in the i/o thread
int EventLoop::handle_pending_add(Channel *channel) 
{
    printf("add channel fd == %d, %s \n", channel->fd(), this->thread_name.c_str());
    return eventDispatcher->add(channel);
}

// in the i/o thread
int EventLoop::handle_pending_remove(Channel *channel) 
{
    printf("remove channel fd == %d, %s \n", channel->fd(), this->thread_name.c_str());
    return eventDispatcher->del(channel);
}

// in the i/o thread
int EventLoop::handle_pending_update(Channel *channel) 
{
    printf("update channel fd == %d, %s \n", channel->fd(), thread_name.c_str());
    return eventDispatcher->update(channel);
}


void EventLoop::wakeup() 
{
    char one = 'a';
    ssize_t n = write(this->socketPair[0], &one, sizeof one);
    if (n != sizeof(one)) {
        printf("[error]: wakeup event loop thread failed\n");
    }
}

void EventLoop::handle_wakeup() 
{
    char one;
    ssize_t n = read(socketPair[1], &one, sizeof(one));
    if (n != sizeof(one)) {
        printf("[error] handleWakeup  failed \n");
    }
    printf("wakeup, %s \n", thread_name.c_str());
}

EventLoop::EventLoop() 
    : EventLoop("")
{
}

EventLoop::~EventLoop()
{
    close(socketPair[0]);
    close(socketPair[1]);
}

EventLoop::EventLoop(const std::string &thread_name)
    : quit(0)
    , thread_name(thread_name.empty() ? "main thread" : thread_name)
    , owner_thread_id(std::this_thread::get_id())
    , is_handle_pending(0)
    , eventDispatcher(new EventDispatcher)
{
    //add the socketfd to event
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketPair) < 0) {
        perror("socketpair set fialed");
    }
    pending_list.reserve(64);
    
    // TODO: 管理 Channle 的内存; 在析构时正确释放
    auto wakeup_chan = ChannelAdaptor::create(socketPair[1], EPOLLIN, this)->read(&EventLoop::handle_wakeup, this);
    printf("wakeup fd == %d\n", wakeup_chan->fd());
    add_channel(wakeup_chan);
}

/**
 *
 * 1.参数验证
 * 2.调用dispatcher来进行事件分发,分发完回调事件处理函数
 */
int EventLoop::run() 
{
    assert_in_same_thread();

    printf("event loop run, %s\n", this->thread_name.c_str());
    struct timeval timeval;
    timeval.tv_sec = 1;

    while (!this->quit) {
        //block here to wait I/O event, and get active channels
        eventDispatcher->dispatch(&timeval);

        //handle the pending channel
        handle_pending_channel();
    }

    printf("event loop end, %s\n", this->thread_name.c_str());
    return 0;
}

void EventLoop::stop() 
{
    quit = 1;
    if (!is_in_same_thread())
        wakeup();
}

void EventLoop::assert_in_same_thread()
{
    if (owner_thread_id != std::this_thread::get_id())
        printf("warning: no in same thread");

    assert(owner_thread_id == std::this_thread::get_id());
}

int EventLoop::is_in_same_thread()
{
    return owner_thread_id == std::this_thread::get_id();
}

} // namespace simpleweb

