#include "IDAllocator.h"

int64_t UIDAllocator::Allocate() {
    if (!queue_.empty()) {
        const int64_t id = queue_.front();
        queue_.pop();
        return id;
    }
    return next_++;
}

int64_t UIDAllocator::AllocateConcurrent() {
    std::unique_lock lock(mutex_);
    if (!queue_.empty()) {
        const int64_t id = queue_.front();
        queue_.pop();
        return id;
    }
    return next_++;
}

void UIDAllocator::Recycle(const int64_t id) {
    queue_.push(id);
}

void UIDAllocator::RecycleConcurrent(const int64_t id) {
    std::unique_lock lock(mutex_);
    queue_.push(id);
}
