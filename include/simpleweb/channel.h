#if !defined(SIMPLEWEB_CHANNEL_INCLUDED)
#define SIMPLEWEB_CHANNEL_INCLUDED


#include <stdint.h>
#include <functional>
#include <memory>


namespace simpleweb {


class EventLoop;


class Channel
{
public:
    using ChannlePtr = std::shared_ptr<Channel>;

    Channel(int fd, uint32_t ev, EventLoop *loop) : fd_{fd}, events_(ev), loop_(loop)
    {
    }

    int fd() const 
    {
        return fd_;
    }

    uint32_t events() const 
        /// 返回监听的事件集
    {
        return events_;
    }

    int is_enabled_write() const;
        /// 返回是否监听写事件

    void enable_write();
        /// 启用监听写事件

    void disable_write();
        /// 关闭写事件监听

    void close_fd();
        /// 关闭 fd 文件描述符；若不使用该函数，析构时不会释放 fd 

    virtual ~Channel();

    friend class EventLoop;
    friend class EventDispatcher;
protected:
    virtual void read() {}
    virtual void write() {}
    virtual void error() {}
    virtual void disconnect() {}

private:
    Channel(const Channel &) = delete;
    Channel& operator = (const Channel &) = delete;

    int fd_ {-1};
    uint32_t events_;
    EventLoop *loop_{nullptr};
};


class ChannelAdaptor : public Channel
    /// Channel 类型的适配器
    /// 在不希望写一个 Channel 派生类时，这是一个简易的适配方法
{
public:
    using Channel::Channel;

    using event_callback = std::function<void ()>;

    static ChannelAdaptor* create(int fd, uint32_t ev, EventLoop *loop)
        /// 这是一个用于创建 ChannelAdaptor 对象的供车方法，主要是希望用于链式调用
        /// example: Channel *chan = ChannelAdaptor::create(...)->read(...)->write(...)->error(...);
    {
        return new ChannelAdaptor(fd, ev, loop);
    }

    template <typename F, typename ...Args>
    ChannelAdaptor* read(F &&f, Args&& ...args)
    {
        read_fn_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return this;
    }

    template <typename F, typename ...Args>
    ChannelAdaptor* write(F &&f, Args&& ...args)
    {
        write_fn_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return this;
    }

    template <typename F, typename ...Args>
    ChannelAdaptor* error(F &&f, Args&& ...args)
    {
        error_fn_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return this;
    }

    template <typename F, typename ...Args>
    ChannelAdaptor* disconnect(F &&f, Args&& ...args)
    {
        disconn_fn_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return this;
    }

    ~ChannelAdaptor() override {};
protected:
    void read() override
    { 
        if (read_fn_) read_fn_(); 
    }

    void write() override
    { 
        if (write_fn_) write_fn_(); 
    }
    
    void error() override
    {
        if (error_fn_) error_fn_();
    }

    void disconnect() override
    {
        if (disconn_fn_) disconn_fn_();
    }
private:
    event_callback read_fn_;
    event_callback write_fn_;
    event_callback error_fn_;
    event_callback disconn_fn_;
};


} // namespace simpleweb


#endif // SIMPLEWEB_CHANNEL_INCLUDED
