#pragma once

#include "safe_container/queue_safe.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace cpputils {
class ThreadPool {
public:
    using size_type = size_t;

public:
    ThreadPool(const size_type threads) : shutdown_(false), threads_(threads) {
        for (size_type i = 0; i < threads; ++i)
            threads_[i] = std::thread(Worker(this, i));
    }
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(const ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;
    ~ThreadPool() {
        shutdown_ = true;
        cv_.notify_all();
        for (size_type i = 0; i < threads_.size(); ++i) {
            if (threads_[i].joinable()) threads_[i].join();
        }
    }

    template <typename Func, typename... Args>
    auto submit(Func &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func =
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
        auto taskptr =
            std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void()> wrapper = [taskptr]() { (*taskptr)(); };
        queue_.enqueue(wrapper);
        cv_.notify_one();
        return taskptr->get_future();
    }

private:
    class Worker {
    public:
        using id_type = size_t;

    public:
        Worker(ThreadPool *pool, const id_type id) : id_(id), pool_(pool) {}
        // Worker() = delete;
        // Worker(const Worker &) = delete;
        // Worker &operator=(const Worker &) = delete;

    public:
        void operator()() {
            bool has_task;
            std::function<void()> func;
            while (!pool_->shutdown_) {
                {
                    std::unique_lock<std::mutex> lock(pool_->mutex_);
                    if (pool_->queue_.empty()) pool_->cv_.wait(lock);
                    has_task = pool_->queue_.dequeue(func);
                }
                if (has_task) func();
            }
        }

    private:
        id_type id_;
        ThreadPool *pool_;
    };

    std::atomic_bool shutdown_;
    std::vector<std::thread> threads_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    QueueSafe<std::function<void()>> queue_;
};
} // namespace cpputils