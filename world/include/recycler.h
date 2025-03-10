#pragma once

#include "poolable.h"
#include "utils.h"

#include <absl/container/flat_hash_set.h>
#include <queue>
#include <shared_mutex>
#include <atomic>


class IRecycler {

    std::queue<IPoolable *> queue_;
    absl::flat_hash_set<IPoolable *> usingSet_;

    mutable std::shared_mutex mutex_;

    std::atomic<ATimePoint> collectTime_;
    size_t minCapacity_;

    float expanseRate_;
    float expanseScale_;

    float collectRate_;
    float collectScale_;

protected:
    IPoolable *acquireInternal();

    void expanse();
    void collect();

public:
    IRecycler();
    virtual ~IRecycler();

    DISABLE_COPY_MOVE(IRecycler)

    [[nodiscard]] size_t capacity() const;

    virtual void init(size_t capacity) = 0;
    void recycle(IPoolable *obj);

    virtual IPoolable* create() const = 0;
};
