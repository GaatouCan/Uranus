#pragma once

#include "Recyclable.h"
#include "Types.h"

#include <queue>
#include <concepts>
#include <atomic>
#include <shared_mutex>

/**
 * The Abstract Base Class Of Recycler,
 * Its Subclass Must Be Created By std::make_shared
 */
class BASE_API IRecycler : public std::enable_shared_from_this<IRecycler> {

    /** asio::io_context Reference For Shrink Timer */
    io_context &mContext;

    /** Internal Container */
    std::queue<std::unique_ptr<IRecyclable>>    mQueue;
    mutable std::shared_mutex                   mMutex;

    std::atomic_int64_t                         mUsage;

    /** Shrink Timer */
    shared_ptr<ASteadyTimer>                    mShrinkTimer;

    /** Expand Flag */
    std::atomic_bool                            bExpanding;

    static constexpr float      RECYCLER_EXPAND_THRESHOLD   = 0.75f;
    static constexpr float      RECYCLER_EXPAND_RATE        = 1.f;

    static constexpr int        RECYCLER_SHRINK_DELAY       = 1;
    static constexpr float      RECYCLER_SHRINK_THRESHOLD   = 0.3f;
    static constexpr float      RECYCLER_SHRINK_RATE        = 0.5f;
    static constexpr int        RECYCLER_MINIMUM_CAPACITY   = 64;

protected:
    explicit IRecycler(io_context &ctx);

    [[nodiscard]] virtual IRecyclable *Create() const = 0;

public:
    IRecycler() = delete;
    virtual ~IRecycler();

    DISABLE_COPY_MOVE(IRecycler)

    void Initial(size_t capacity = 64);

    /** Pop The Element Of The Front Of The Internal Queue */
    std::shared_ptr<IRecyclable> Acquire();

    [[nodiscard]] size_t GetUsage() const;
    [[nodiscard]] size_t GetIdle() const;
    [[nodiscard]] size_t GetCapacity() const;

private:
    void Recycle(IRecyclable *elem);

    void Expand();
    void Shrink();
};


template<class Type>
requires std::derived_from<Type, IRecyclable> && (!std::is_same_v<Type, IRecyclable>)
class TRecycler final : public IRecycler {

protected:
    IRecyclable *Create() const override {
        return new Type();
    }

public:
    explicit TRecycler(asio::io_context &ctx)
        : IRecycler(ctx) {
    }

    std::shared_ptr<Type> AcquireT() {
        auto res = this->Acquire();
        if (res == nullptr)
            return nullptr;

        return std::dynamic_pointer_cast<Type>(res);
    }
};
