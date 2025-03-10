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
    size_t minCapacity_;

    float expanseRate_;
    float expanseScale_;

    float collectRate_;
    float collectScale_;

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
