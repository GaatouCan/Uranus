#pragma once

#include "recyclable.h"
#include "utils.h"

#include <absl/container/flat_hash_set.h>
#include <queue>
#include <shared_mutex>
#include <atomic>
#include <concepts>


class BASE_API IRecycler {

    std::queue<IRecyclable *> queue_;
    absl::flat_hash_set<IRecyclable *> usingSet_;

    mutable std::shared_mutex mutex_;

    std::atomic<ATimePoint> collectTime_;
    size_t minCapacity_;    // 最小容量

    float expanseRate_;     // 已使用到该比例时扩展
    float expanseScale_;    // 扩展倍数

    float collectRate_;     // 未使用小于该比例时回收
    float collectScale_;    // 回收总数百分比

protected:
    void expanse();
    void collect();

public:
    IRecycler();
    virtual ~IRecycler();

    DISABLE_COPY_MOVE(IRecycler)

    [[nodiscard]] size_t capacity() const;

    IRecycler &setMinimumCapacity(size_t capacity);

    IRecycler &setExpanseRate(float rate);
    IRecycler &setExpanseScale(float scale);

    IRecycler &setCollectRate(float rate);
    IRecycler &setCollectScale(float scale);

    virtual void init(size_t capacity);

    IRecyclable *acquire();
    void recycle(IRecyclable *obj);

    virtual IRecyclable *create() = 0;
};


template<class Type>
requires std::derived_from<Type, IRecyclable>
class BASE_API TRecycler final : public IRecycler {
public:
    Type *acquireT() {
        auto res = acquire();
        return dynamic_cast<Type *>(res);
    }

    IRecyclable *create() override {
        return new Type(this);
    }
};
