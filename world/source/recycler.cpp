#include "../include/recycler.h"


IRecycler::IRecycler()
    : capacity_(64),
      expanseScale_(1.f) {
}

IRecycler::~IRecycler() {
    for (const auto &val: usingSet_) {
        delete val;
    }

    while (!queue_.empty()) {
        const auto elem = queue_.front();
        queue_.pop();
        delete elem;
    }
}

IRecyclable *IRecycler::acquire() {
    IRecyclable *res = nullptr;

    {
        std::unique_lock lock(mutex_);
        if (queue_.empty()) {
            const auto num = static_cast<size_t>(std::ceil(static_cast<float>(usingSet_.size()) * expanseScale_));

            for (auto idx = 0; idx < num; ++idx)
                queue_.emplace(create());

            capacity_ += num;
        }

        res = queue_.front();
        queue_.pop();
        usingSet_.insert(res);
    }

    res->initial();
    assert(res->unused());

    return res;
}

size_t IRecycler::capacity() const {
    return capacity_;
}

IRecycler & IRecycler::setCapacity(const size_t capacity) {
    if (queue_.empty() && usingSet_.empty())
        capacity_ = capacity;

    return *this;
}

IRecycler &IRecycler::setExpanseScale(const float scale) {
    expanseScale_ = scale;
    return *this;
}


void IRecycler::init() {
    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < capacity_; i++) {
        queue_.emplace(create());
    }
}

void IRecycler::shrink(const size_t rest) {
    if (rest == 0) {
        std::unique_lock lock(mutex_);

        while (!queue_.empty()) {
            const auto elem = queue_.front();
            queue_.pop();
            delete elem;
        }

        capacity_ = usingSet_.size();
        return;
    }

    auto num = queue_.size() - rest;

    std::unique_lock lock(mutex_);

    while (num > 0 && !queue_.empty()) {
        const auto elem = queue_.front();
        queue_.pop();
        delete elem;

        num--;
    }

    capacity_ = queue_.size() + usingSet_.size();
}

void IRecycler::recycle(IRecyclable *obj) {
    if (obj == nullptr)
        return;

    if (obj->handle_ != nullptr && obj->handle_ != this) {
        obj->recycle();
        return;
    }

    obj->reset();

    std::unique_lock lock(mutex_);
    if (usingSet_.erase(obj) > 0) {
        // obj->reset();
        queue_.emplace(obj);
    }
}
