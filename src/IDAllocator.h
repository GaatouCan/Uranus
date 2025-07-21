#pragma once

#include "Common.h"

#include <queue>
#include <atomic>
#include <mutex>

class BASE_API UIDAllocator {

public:
    int64_t Allocate();
    int64_t AllocateConcurrent();

    void Recycle(int64_t id);
    void RecycleConcurrent(int64_t id);

private:
    std::queue<int64_t> queue_;
    std::mutex mutex_;
    std::atomic_int64_t next_;
};