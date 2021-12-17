#if !defined(SIMPLEWEB_THREAD_POOL_INCLUDED)
#define SIMPLEWEB_THREAD_POOL_INCLUDED


#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <future>
#include <functional>


namespace simpleweb {
    

class thread_pool
{
public:
    thread_pool(size_t max = std::thread::hardware_concurrency())
    {
        for (size_t i = 0; i < max; i++)
        {
            workers_.emplace_back([this] {
                while(true)
                {
                    std::function<void ()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->mutex_);
                        this->cond_.wait(lock, [this] { return this->stop_ || !this->tasks_.empty(); });
                        if(this->stop_ && this->tasks_.empty())
                            return;
                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }

                    task();
                }
            });
        }
    }
    
    void enqueue(std::function<void ()> task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
        cond_.notify_one();
    }

    void shutdown()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cond_.notify_all();

        for (auto &w : workers_) 
            w.join();

        workers_.clear();
    }

    ~thread_pool() = default;
private:
    std::vector<std::thread> workers_;
    bool stop_ {false};
    std::queue<std::function<void ()>> tasks_;
    std::condition_variable cond_;
    std::mutex mutex_;
};


} // namespace simpleweb


#endif // SIMPLEWEB_THREAD_POOL_INCLUDED
