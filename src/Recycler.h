#pragma once

#include "RecycleInterface.h"
#include "Types.h"

#include <queue>
#include <concepts>
#include <atomic>
#include <shared_mutex>

/**
 * The Abstract Base Class Of Recycler,
 * Its Subclass Must Be Created By std::make_shared
 */
class BASE_API IRecyclerBase : public std::enable_shared_from_this<IRecyclerBase> {

    /** asio::io_context Reference For Shrink Timer */
    io_context &IOContext;

    /** Internal Container */
    std::queue<unique_ptr<IRecycleInterface>> InnerQueue;
    mutable std::shared_mutex Mutex;

    std::atomic_int64_t Usage;

    /** Shrink Timer */
    shared_ptr<ASteadyTimer> ShrinkTimer;

    /** Expand Flag */
    std::atomic_bool bExpanding;

    static constexpr float      RECYCLER_EXPAND_THRESHOLD   = 0.75f;
    static constexpr float      RECYCLER_EXPAND_RATE        = 1.f;

    static constexpr int        RECYCLER_SHRINK_DELAY       = 1;
    static constexpr float      RECYCLER_SHRINK_THRESHOLD   = 0.3f;
    static constexpr float      RECYCLER_SHRINK_RATE        = 0.5f;
    static constexpr int        RECYCLER_MINIMUM_CAPACITY   = 64;

protected:
    explicit IRecyclerBase(io_context &ctx);

    [[nodiscard]] virtual IRecycleInterface *Create() const = 0;

public:
    IRecyclerBase() = delete;
    virtual ~IRecyclerBase();

    DISABLE_COPY_MOVE(IRecyclerBase)

    void Initial(size_t capacity = 64);

    /** Pop The Element Of The Front Of The Internal Queue */
    std::shared_ptr<IRecycleInterface> Acquire();

    [[nodiscard]] size_t GetUsage() const;
    [[nodiscard]] size_t GetIdle() const;
    [[nodiscard]] size_t GetCapacity() const;

private:
    void Recycle(IRecycleInterface *elem);

    void Expand();
    void Shrink();
};


template<class Type>
requires std::derived_from<Type, IRecycleInterface> && (!std::is_same_v<Type, IRecycleInterface>)
class TRecycler final : public IRecyclerBase {

protected:
    IRecycleInterface *Create() const override {
        return new Type();
    }

public:
    explicit TRecycler(io_context &ctx)
        : IRecyclerBase(ctx) {
    }

    shared_ptr<Type> AcquireT() {
        auto res = this->Acquire();
        if (res == nullptr)
            return nullptr;

        return std::dynamic_pointer_cast<Type>(res);
    }
};
