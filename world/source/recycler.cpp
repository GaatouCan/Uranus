#include "../include/recycler.h"


IRecycler::IRecycler()
    : capacity_(64),
      expanseScale_(1.f) {
}

IRecycler::~IRecycler() {
    for (const auto &val: usingSet_) {
        delete val;
    }

    for (const auto &val : pool_) {
        delete val;
    }
}

IRecyclable *IRecycler::acquire() {
    IRecyclable *res = nullptr;

    {
        std::unique_lock lock(mutex_);
        if (pool_.empty()) {
            const auto num = static_cast<size_t>(std::ceil(static_cast<float>(usingSet_.size()) * expanseScale_));

            for (auto idx = 0; idx < num; ++idx)
                pool_.emplace_back(create());

            capacity_ += num;
        }

        res = pool_.back();
        pool_.pop_back();
        usingSet_.insert(res);
    }

    res->initial();
    assert(res->unused());

    return res;
}

size_t IRecycler::capacity() const {
    return capacity_;
}

IRecycler & IRecycler::setCapacity(size_t capacity) {
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
        pool_.emplace_back(create());
    }
}

void IRecycler::shrink(const size_t rest) {
    if (rest == 0) {
        std::unique_lock lock(mutex_);
        for (const auto &val: pool_)
            delete val;

        pool_.clear();

        capacity_ = usingSet_.size();

        return;
    }

    auto num = pool_.size() - rest;

    std::unique_lock lock(mutex_);

    while (num > 0 && !pool_.empty()) {
        const auto elem = pool_.back();
        pool_.pop_back();

        delete elem;
        num--;
    }

    capacity_ = pool_.size() + usingSet_.size();
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
        pool_.emplace_back(obj);
    }
}
