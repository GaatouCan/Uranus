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

    T &Front() {
        std::shared_lock lock(mtx_);
        return deque_.front();
    }

    T &Back() {
        std::shared_lock lock(mtx_);
        return deque_.back();
    }

    void PushFront(const T &data) {
        {
            std::unique_lock lock(mtx_);
            deque_.push_front(data);
        }

        cv_.notify_one();
    }

    void PushBack(const T &data) {
        {
            std::unique_lock lock(mtx_);
            deque_.push_back(data);
        }

        cv_.notify_one();
    }

    void PushFront(T &&data) {
        {
            std::unique_lock lock(mtx_);
            deque_.emplace_front(data);
        }

        cv_.notify_one();
    }

    void PushBack(T &&data) {
        {
            std::unique_lock lock(mtx_);
            deque_.emplace_back(data);
        }

        cv_.notify_one();
    }

    std::optional<T> PopFront() {
        std::unique_lock lock(mtx_);

        if (deque_.empty())
            return std::nullopt;

        auto data = std::move(deque_.front());
        deque_.pop_front();

        return data;
    }

    std::optional<T> PopBack() {
        std::unique_lock lock(mtx_);
        if (deque_.empty())
            return std::nullopt;

        auto data = std::move(deque_.back());
        deque_.pop_back();

        return data;
    }

    bool IsEmpty() const {
        std::shared_lock lock(mtx_);
        return deque_.empty();
    }

    size_t Size() const {
        std::shared_lock lock(mtx_);
        return deque_.size();
    }

    void Clear() {
        std::unique_lock blockLock(mtx_);
        deque_.clear();
    }

    void Quit() {
        quit_ = true;
        cv_.notify_all();
    }

    bool IsRunning() const {
        return !quit_;
    }

    void Wait() {
        std::unique_lock lock(mtx_);
        cv_.wait(lock, [this] { return !deque_.empty() || quit_; });
    }

private:
    std::deque<T> deque_;

    mutable std::shared_mutex mtx_;
    std::condition_variable_any cv_;

    std::atomic_bool quit_{false};
};
