#pragma once

#include <deque>
#include <condition_variable>
#include <shared_mutex>
#include <atomic>
#include <optional>

#include "common.h"


template<typename T>
class BASE_API TDeque {
public:
    TDeque() = default;
    ~TDeque() = default;

    T &front() {
        std::shared_lock lock(mutex_);
        return deque_.front();
    }

    T &back() {
        std::shared_lock lock(mutex_);
        return deque_.back();
    }

    void pushFront(const T &data) {
        {
            std::unique_lock lock(mutex_);
            deque_.push_front(data);
        }

        conditionVariable_.notify_one();
    }

    void pushBack(const T &data) {
        {
            std::unique_lock lock(mutex_);
            deque_.push_back(data);
        }

        conditionVariable_.notify_one();
    }

    void pushFront(T &&data) {
        {
            std::unique_lock lock(mutex_);
            deque_.emplace_front(data);
        }

        conditionVariable_.notify_one();
    }

    void pushBack(T &&data) {
        {
            std::unique_lock lock(mutex_);
            deque_.emplace_back(data);
        }

        conditionVariable_.notify_one();
    }

    std::optional<T> popFront() {
        std::unique_lock lock(mutex_);

        if (deque_.empty())
            return std::nullopt;

        auto data = std::move(deque_.front());
        deque_.pop_front();

        return data;
    }

    std::optional<T> popBack() {
        std::unique_lock lock(mutex_);
        if (deque_.empty())
            return std::nullopt;

        auto data = std::move(deque_.back());
        deque_.pop_back();

        return data;
    }

    bool empty() const {
        std::shared_lock lock(mutex_);
        return deque_.empty();
    }

    size_t size() const {
        std::shared_lock lock(mutex_);
        return deque_.size();
    }

    void clear() {
        std::unique_lock blockLock(mutex_);
        deque_.clear();
    }

    void quit() {
        quit_ = true;
        conditionVariable_.notify_all();
    }

    bool running() const {
        return !quit_;
    }

    void wait() {
        std::unique_lock lock(mutex_);
        conditionVariable_.wait(lock, [this] { return !deque_.empty() || quit_; });
    }

private:
    std::deque<T> deque_;

    mutable std::shared_mutex mutex_;
    std::condition_variable_any conditionVariable_;

    std::atomic_bool quit_{false};
};
