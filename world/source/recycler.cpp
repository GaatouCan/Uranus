#include "../include/recycler.h"

IRecycler::IRecycler()
    : minCapacity_(64),
      expanseRate_(0.3f),
      expanseScale_(1.f),
      collectRate_(1.f),
      collectScale_(0.7f) {
}

IRecycler::~IRecycler() {
    for (const auto it: usingSet_) {
        delete it;
    }

    while (!queue_.empty()) {
        const auto elem = queue_.front();
        queue_.pop();
        delete elem;
    }
}

IPoolable *IRecycler::acquireInternal() {
    expanse();

    IPoolable *res = nullptr; {
        std::unique_lock lock(mutex_);
        res = queue_.front();
        queue_.pop();
        usingSet_.insert(res);
    }

    res->initial();
    return res;
}

size_t IRecycler::capacity() const {
    std::shared_lock lock(mutex_);
    return queue_.size() + usingSet_.size();
}

void IRecycler::init(const size_t capacity) {
    for (size_t i = 0; i < capacity; i++) {
        auto *elem = create();;

        if (elem != nullptr) {
            queue_.push(elem);
        }
    }
}

void IRecycler::expanse() {
    if (usingSet_.empty() && !queue_.empty())
        return;

    if (std::floor(queue_.size() / capacity()) <= expanseRate_)
        return;

    const auto num = static_cast<size_t>(std::ceil(static_cast<float>(capacity()) * expanseScale_));

    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < num; i++) {
        if (auto *elem = create(); elem != nullptr) {
            queue_.push(elem);
        }
    }
}

void IRecycler::collect() {
    const auto now = NowTimePoint();

    // 不要太频繁
    if (now - collectTime_.load() < std::chrono::seconds(3))
        return;

    {
        std::unique_lock lock(mutex_);
        erase_if(usingSet_, [this](const auto &it) {
            if (!it->available()) {
                it->reset();
                queue_.push(it);
                return true;
            }
            return false;
        });
    }

    if (queue_.size() <= minCapacity_ || std::floor(queue_.size() / capacity()) < collectRate_)
        return;

    collectTime_ = now;

    const auto num = static_cast<size_t>(std::floor(static_cast<float>(capacity()) * collectScale_));

    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < num && queue_.size() > minCapacity_; i++) {
        const auto elem = queue_.front();
        queue_.pop();
        delete elem;
    }
}

void IRecycler::recycle(IPoolable *obj) {
    if (obj == nullptr)
        return;

    if (obj->handle_ != nullptr && obj->handle_ != this) {
        obj->recycle();
        return;
    } {
        std::shared_lock lock(mutex_);
        if (!usingSet_.contains(obj))
            return;
    }

    if (obj->available())
        obj->reset(); {
        std::unique_lock lock(mutex_);

        queue_.push(obj);
        usingSet_.erase(obj);
    }

    collect();
}
