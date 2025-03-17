#pragma once

#include "recyclable.h"
#include "utils.h"

#include <absl/container/flat_hash_set.h>
#include <queue>
#include <atomic>
#include <concepts>
#include <functional>
#include <shared_mutex>


class BASE_API IRecycler {

    std::queue<IRecyclable *> queue_;
    absl::flat_hash_set<IRecyclable *> usingSet_;

    mutable std::shared_mutex mutex_;

    std::atomic_size_t capacity_;    // 默认容量
    float expanseScale_;             // 扩展倍数

protected:
    virtual IRecyclable *create() = 0;

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

    void shrink(size_t rest = 0);
};


template<class Type>
requires std::derived_from<Type, IRecyclable>
class BASE_API TRecycler final : public IRecycler {

    std::function<void(Type *)> onCreate_;

public:
    void setCreateCallback(const std::function<void(Type *)> &cb) {
        onCreate_ = cb;
    }

    Type *acquireT() {
        auto res = acquire();
        return dynamic_cast<Type *>(res);
    }

private:
    IRecyclable *create() override {
        auto elem = new Type(this);

        if (onCreate_)
            onCreate_(elem);

        return elem;
    }
};
