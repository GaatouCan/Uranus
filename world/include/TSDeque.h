#pragma once

#include <deque>
#include <condition_variable>
#include <shared_mutex>
#include <atomic>
#include <optional>

/**
 * Thread Safe Deque
 * @tparam T Element Type
 */
template<typename T>
class BASE_API TSDeque {
public:
    TSDeque() = default;
    ~TSDeque() = default;

    T &Front() {
        std::shared_lock lock(mMutex);
        return mDeque.front();
    }

    T &Back() {
        std::shared_lock lock(mMutex);
        return mDeque.back();
    }

    void PushFront(const T &data) {
        {
            std::unique_lock lock(mMutex);
            mDeque.push_front(data);
        }

        mCondVar.notify_one();
    }

    void PushBack(const T &data) {
        {
            std::unique_lock lock(mMutex);
            mDeque.push_back(data);
        }

        mCondVar.notify_one();
    }

    void PushFront(T &&data) {
        {
            std::unique_lock lock(mMutex);
            mDeque.emplace_front(data);
        }

        mCondVar.notify_one();
    }

    void PushBack(T &&data) {
        {
            std::unique_lock lock(mMutex);
            mDeque.emplace_back(data);
        }

        mCondVar.notify_one();
    }

    std::optional<T> PopFront() {
        std::unique_lock lock(mMutex);

        if (mDeque.empty())
            return std::nullopt;

        auto data = std::move(mDeque.front());
        mDeque.pop_front();

        return data;
    }

    std::optional<T> PopBack() {
        std::unique_lock lock(mMutex);
        if (mDeque.empty())
            return std::nullopt;

        auto data = std::move(mDeque.back());
        mDeque.pop_back();

        return data;
    }

    bool IsEmpty() const {
        std::shared_lock lock(mMutex);
        return mDeque.empty();
    }

    size_t Size() const {
        std::shared_lock lock(mMutex);
        return mDeque.size();
    }

    void Clear() {
        std::unique_lock blockLock(mMutex);
        mDeque.clear();
    }

    void Quit() {
        bQuit = true;
        mCondVar.notify_all();
    }

    bool IsRunning() const {
        return !bQuit;
    }

    void Wait() {
        std::unique_lock lock(mMutex);
        mCondVar.wait(lock, [this] { return !mDeque.empty() || bQuit; });
    }

private:
    std::deque<T> mDeque;

    mutable std::shared_mutex mMutex;
    std::condition_variable_any mCondVar;

    std::atomic_bool bQuit{false};
};
