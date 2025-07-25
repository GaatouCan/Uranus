﻿#include "Recycler.h"

#include <spdlog/spdlog.h>
#ifdef __linux__
#include <mutex>
#endif


IRecyclerBase::IRecyclerBase(io_context &ctx)
    : mContext(ctx),
      mUsage(-1),
      mShrinkTimer(nullptr),
      bExpanding(false) {

    static_assert(RECYCLER_SHRINK_RATE > RECYCLER_SHRINK_THRESHOLD);
}

IRecyclerBase::~IRecyclerBase() {
    if (mShrinkTimer != nullptr) {
        mShrinkTimer->cancel();
    }
}

std::shared_ptr<IRecycleInterface> IRecyclerBase::Acquire() {
    // Not Initialized
    if (mUsage < 0)
        return nullptr;

    // Custom Deleter Of The Smart Pointer
    auto deleter = [weak = weak_from_this()](IRecycleInterface *pElem) {
        if (const auto self = weak.lock()) {
            self->Recycle(pElem);
        } else {
            delete pElem;
        }
    };

    IRecycleInterface *pObject = nullptr;

    // Pop The Front From The Queue If It Is Not Empty
    {
        std::unique_lock lock(mMutex);
        if (!mQueue.empty()) {
            auto ptr = std::move(mQueue.front());
            mQueue.pop();

            pObject = ptr.release();

            SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Acquire Recyclable[{:p}] From Queue",
                __FUNCTION__, static_cast<void *>(this), static_cast<void *>(pObject));
        }
    }

    // Not Available Element In The Queue, Create New One And Return The New
    if (pObject == nullptr) {
        pObject = Create();
        pObject->OnCreate();

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Acquire New Recyclable[{:p}]",
            __FUNCTION__, static_cast<void *>(this), static_cast<void *>(pObject));
    }

    pObject->Initial();
    ++mUsage;

    if (!bExpanding) {
        std::shared_lock lock(mMutex);

        // Check If It Needs To Expand
        if (static_cast<float>(mUsage.load()) >= std::ceil(static_cast<float>(mQueue.size() + mUsage.load()) * RECYCLER_EXPAND_THRESHOLD)) {
            bExpanding = true;
            co_spawn(mContext, [weak = weak_from_this()]() -> awaitable<void> {
                if (const auto self = weak.lock()) {
                    self->Expand();
                }
                co_return;
            }, detached);
        }
    }

    return { pObject, deleter };
}

size_t IRecyclerBase::GetUsage() const {
    if (mUsage < 0)
        return 0;
    return mUsage;
}

size_t IRecyclerBase::GetIdle() const {
    if (mUsage < 0)
        return 0;

    std::shared_lock lock(mMutex);
    return mQueue.size();
}

size_t IRecyclerBase::GetCapacity() const {
    if (mUsage < 0)
        return 0;

    std::shared_lock lock(mMutex);
    return mQueue.size() + mUsage.load();
}

void IRecyclerBase::Shrink() {
    if (bExpanding) {
        mShrinkTimer.reset();
        return;
    }

    size_t num = 0;

    // Check If It Needs To Shrink
    {
        std::shared_lock lock(mMutex);

        // Recycler Total Capacity
        const size_t total = mQueue.size() + mUsage.load();

        // Usage Less Than Shrink Threshold
        if (static_cast<float>(mUsage.load()) < std::ceil(static_cast<float>(total) * RECYCLER_SHRINK_THRESHOLD)) {
            num = static_cast<size_t>(std::floor(static_cast<float>(total) * RECYCLER_SHRINK_RATE));

            const auto rest = total - num;
            if (rest <= 0) {
                mShrinkTimer.reset();
                return;
            }

            if (rest < RECYCLER_MINIMUM_CAPACITY) {
                num = total - RECYCLER_MINIMUM_CAPACITY;
            }
        }
    }

    if (num > 0) {
        SPDLOG_TRACE("{:<20} - Recycler[{:p}] Need To Release {} Elements",
            __FUNCTION__, static_cast<void *>(this), num);

        std::unique_lock lock(mMutex);

        while (num-- > 0 && !mQueue.empty()) {
            const auto elem = std::move(mQueue.front());
            mQueue.pop();
        }

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] Shrink Finished",
            __FUNCTION__, static_cast<void *>(this));
    }

    mShrinkTimer.reset();
}

void IRecyclerBase::Recycle(IRecycleInterface *pElem) {
    if (mUsage < 0) {
        delete pElem;
        return;
    }

    pElem->Reset();
    --mUsage;

    {
        std::unique_lock lock(mMutex);
        mQueue.emplace(pElem);

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Recycle Recyclable[{:p}] To Queue",
            __FUNCTION__, static_cast<void *>(this), static_cast<void *>(pElem));
    }

    if (mShrinkTimer != nullptr || bExpanding)
        return;

    // Do Shrink Later
    mShrinkTimer = make_shared<ASteadyTimer>(mContext);

    co_spawn(mContext, [weak = weak_from_this(), timer = mShrinkTimer]() mutable -> awaitable<void> {
        timer->expires_after(std::chrono::seconds(RECYCLER_SHRINK_DELAY));

        // If Shrink Timer Be Canceled, It Will Not Shrink
        if (const auto [ec] = co_await timer->async_wait(); !ec) {
            if (const auto self = weak.lock()) {
                self->Shrink();
            }
        }
    }, detached);
}

void IRecyclerBase::Expand() {
    // Cancel The Shrink Timer
    if (mShrinkTimer != nullptr) {
        mShrinkTimer->cancel();
    }

    size_t num = 0;
    std::vector<IRecycleInterface *> elems;

    // Calculate How Many New Elements Need To Be Created
    {
        std::shared_lock lock(mMutex);
        const size_t total = mQueue.size() + mUsage.load();
        num = static_cast<size_t>(static_cast<float>(total) * RECYCLER_EXPAND_RATE);
    }

    while (num-- > 0) {
        auto *pElem = Create();
        pElem->OnCreate();

        elems.emplace_back(pElem);
    }

    if (!elems.empty()) {
        std::unique_lock lock(mMutex);
        for (const auto &pElem : elems) {
            mQueue.emplace(pElem);
        }
    }

    bExpanding = false;
}

void IRecyclerBase::Initial(const size_t capacity) {
    if (mUsage >= 0)
        return;

    for (size_t count = 0; count < capacity; count++) {
        auto *pElem = Create();
        pElem->OnCreate();

        mQueue.emplace(pElem);
    }
    mUsage = 0;
    SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Capacity[{}]", __FUNCTION__, static_cast<void *>(this), capacity);
}
