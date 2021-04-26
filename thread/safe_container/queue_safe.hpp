#pragma once

#include <mutex>
#include <queue>

namespace cpputils {
template <typename T>
class QueueSafe {
public:
    using size_type = typename std::queue<T>::size_type;

public:
    QueueSafe() = default;
    QueueSafe(const QueueSafe &) = delete;
    QueueSafe(QueueSafe &&other) {
        std::lock_guard<std::mutex> lock(other.mutex_);
        queue_ = std::move(other.queue_);
    }
    ~QueueSafe() = default;
    QueueSafe &operator=(const QueueSafe &) = delete;
    QueueSafe &operator=(QueueSafe &&other) {
        swap(*this, other);
        return *this;
    }

public:
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_type size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    void enqueue(const T &t) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace(t);
    }

    bool dequeue(T &value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;

        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;

    template <typename U>
    friend void swap(QueueSafe<U> &lhs, QueueSafe<U> &rhs) noexcept;
};

template <typename T>
void swap(QueueSafe<T> &lhs, QueueSafe<T> &rhs) noexcept {
    if (&lhs == &rhs) return;
    std::lock_guard<std::mutex> lhs_lock(lhs.mutex_), rhs_lock(rhs.mutex_);

    std::swap(lhs.queue_, rhs.queue_);
}

} // namespace cpputils
