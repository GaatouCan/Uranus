#pragma once

#include <queue>
#include <atomic>
#include <mutex>

template<bool bConcurrent>
class TIDAllocator {

public:
    int64_t Allocate() {
        if constexpr (bConcurrent) {
            std::unique_lock lock(mutex_);
            if (!queue_.empty()) {
                const auto result = queue_.front();
                queue_.pop();
                return result;
            }
        } else {
            if (!queue_.empty()) {
                const auto result = queue_.front();
                queue_.pop();
                return result;
            }
        }

        return next_++;
    }

    void Recycle(const int64_t id) {
        if constexpr (bConcurrent) {
            std::unique_lock lock(mutex_);
            queue_.push(id);
            return;
        }
        queue_.push(id);
    }

private:
    std::queue<int64_t> queue_;
    std::mutex mutex_;
    std::atomic_int64_t next_;
};