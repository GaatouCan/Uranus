#pragma once

#include <deque>
#include <condition_variable>
#include <shared_mutex>
#include <atomic>

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
        std::scoped_lock lock(mMutex);
        return mDeque.front();
    }

    T &Back() {
        std::scoped_lock lock(mMutex);
        return mDeque.back();
    }

    void PushFront(const T &data) {
        std::scoped_lock lock(mMutex);
        mDeque.push_front(data);

        std::unique_lock blockLock(mBlocking);
        mCondVar.notify_one();
    }

    void PushBack(const T &data) {
        std::scoped_lock lock(mMutex);
        mDeque.push_back(data);

        std::unique_lock blockLock(mBlocking);
        mCondVar.notify_one();
    }

    void PushFront(T &&data) {
        std::scoped_lock lock(mMutex);
        mDeque.emplace_front(data);

        std::unique_lock blockLock(mBlocking);
        mCondVar.notify_one();
    }

    void PushBack(T &&data) {
        std::scoped_lock lock(mMutex);
        mDeque.emplace_back(data);

        std::unique_lock blockLock(mBlocking);
        mCondVar.notify_one();
    }

    T PopFront() {
        std::scoped_lock lock(mMutex);
        auto data = std::move(mDeque.front());
        mDeque.pop_front();
        return data;
    }

    T PopBack() {
        std::scoped_lock lock(mMutex);
        auto data = std::move(mDeque.back());
        mDeque.pop_back();
        return data;
    }

    bool IsEmpty() const {
        std::shared_lock lock(mSharedMutex);
        return mDeque.empty();
    }

    size_t Size() const {
        std::shared_lock lock(mSharedMutex);
        return mDeque.size();
    }

    void Clear() {
        std::scoped_lock lock(mMutex);
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
        std::unique_lock lock(mBlocking);
        mCondVar.wait(lock, [this] { return !mDeque.empty() || bQuit; });
    }

private:
    std::deque<T> mDeque;

    std::mutex mMutex;                      // For Write
    mutable std::shared_mutex mSharedMutex; // For Read

    std::mutex mBlocking;                   // For wait
    std::condition_variable mCondVar;

    std::atomic_bool bQuit{false};
};
