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
    std::lock_guard<std::mutex> lock(mutex_);
    is_handle_pending_ = 1;

    for (auto [channel, type] : pending_list_)
    {
        if (type == 1)
            handle_pending_add(channel);
        else if (type == 2)
            handle_pending_remove(channel);
        else if (type == 3)
            handle_pending_update(channel);
    }
    pending_list_.clear();
    is_handle_pending_ = 0;
    return 0;
}

int EventLoop::do_channel_event(ChannelPtr channel, int type) 
{
    {
        // scope lock
        std::lock_guard<std::mutex> lock(mutex_);
        assert(is_handle_pending_ == 0);
         // 将通道事件入队
        pending_list_.push_back(std::make_pair(channel, type));
    }

    if (!is_in_loop_thread()) 
        wakeup();
    else 
        handle_pending_channel();

    return 0;
}

int EventLoop::add_channel(ChannelPtr channel) 
{
    if (is_in_loop_thread())
        return handle_pending_add(std::move(channel));
    return do_channel_event(std::move(channel), 1);
}

int EventLoop::remove_channel(ChannelPtr channel) 
{
    if (is_in_loop_thread())
        return handle_pending_remove(std::move(channel));
    return do_channel_event(std::move(channel), 2);
}

int EventLoop::update_channel(ChannelPtr channel) 
{
    if (is_in_loop_thread())
        return handle_pending_update(std::move(channel));
    return do_channel_event(std::move(channel), 3);
}

// in the i/o thread
int EventLoop::handle_pending_add(ChannelPtr channel) 
{
    if (!channel || channel->fd() < 0) return -1;
    if (channels_.contains(channel->fd())) {
        printf("重复的 fd = %d \r\n", channel->fd());
        return -1;
    }

    channels_[channel->fd()] = channel;
    printf("add channel fd == %d, %s \n", channel->fd(), this->thread_name.c_str());
    return dispatcher_->add(channel.get());
}

// in the i/o thread
int EventLoop::handle_pending_remove(ChannelPtr channel) 
{
    auto it = channels_.find(channel->fd());
    if (it != channels_.end()) {
        assert(it->second == channel);
        printf("remove channel fd == %d, %s \n", channel->fd(), this->thread_name.c_str());
        dispatcher_->del(channel.get());
        channels_.erase(it);
    }
    return 0;
}

// in the i/o thread
int EventLoop::handle_pending_update(ChannelPtr channel) 
{
    auto it = channels_.find(channel->fd());
    if (it != channels_.end()) {
        assert(it->second == channel);
        printf("update channel fd == %d, %s \n", channel->fd(), thread_name.c_str());
        dispatcher_->update(channel.get());
    }
    return 0;
}

void EventLoop::wakeup() 
{
    char one = 'a';
    ssize_t n = write(this->socketpair_[0], &one, sizeof one);
    if (n != sizeof(one)) {
        printf("[error]: wakeup event loop thread failed\n");
    }
}

void EventLoop::handle_wakeup() 
{
    char one;
    ssize_t n = read(socketpair_[1], &one, sizeof(one));
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
    close(socketpair_[0]);
    close(socketpair_[1]);
}

EventLoop::EventLoop(const std::string &thread_name)
    : quit_(0)
    , thread_name(thread_name.empty() ? "main thread" : thread_name)
    , owner_thread_id(std::this_thread::get_id())
    , is_handle_pending_(0)
    , dispatcher_(new EventDispatcher)
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketpair_) < 0) {
        perror("socketpair set fialed");
    }
    pending_list_.reserve(128);
    
    auto wakeup_chan = std::make_shared<ChannelAdaptor>(socketpair_[1], EPOLLIN, this);
    wakeup_chan->read(&EventLoop::handle_wakeup, this);

    printf("EventLoop wakeup fd == %d\n", wakeup_chan->fd());
    add_channel(wakeup_chan);
}

int EventLoop::run() 
{
    assert_in_loop_thread();

    printf("event loop run, %s\n", this->thread_name.c_str());
    struct timeval timeval;
    timeval.tv_sec = 1;

    while (!this->quit_) {
        //block here to wait I/O event, and get active channels
        dispatcher_->dispatch(&timeval);

        //handle the pending channel
        handle_pending_channel();
    }

    printf("event loop end, %s\n", this->thread_name.c_str());
    return 0;
}

void EventLoop::stop() 
{
    quit_ = 1;
    if (!is_in_loop_thread())
        wakeup();
}

void EventLoop::assert_in_loop_thread()
{
    if (owner_thread_id != std::this_thread::get_id())
        printf("warning: no in same thread");

    assert(owner_thread_id == std::this_thread::get_id());
}

int EventLoop::is_in_loop_thread()
{
    return owner_thread_id == std::this_thread::get_id();
}

} // namespace simpleweb

