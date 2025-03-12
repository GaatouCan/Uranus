#pragma once

#include "recyclable.h"
#include "utils.h"

#include <absl/container/flat_hash_set.h>
#include <queue>
#include <shared_mutex>
#include <atomic>
#include <concepts>
#include <vector>


class BASE_API IRecycler {

    std::vector<IRecyclable *> pool_;
    absl::flat_hash_set<IRecyclable *> usingSet_;

    mutable std::shared_mutex mutex_;

    std::atomic_size_t capacity_;    // 默认容量
    float expanseScale_;        // 扩展倍数

protected:
    void shrink(size_t rest = 0);

public:
    IRecycler();
    virtual ~IRecycler();

    DISABLE_COPY_MOVE(IRecycler)

    [[nodiscard]] size_t capacity() const;

    IRecycler &setCapacity(size_t capacity);
    IRecycler &setExpanseScale(float scale);

    virtual void init();

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
