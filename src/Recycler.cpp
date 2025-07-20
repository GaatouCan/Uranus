#include "Recycler.h"

#include <spdlog/spdlog.h>
#ifdef __linux__
#include <mutex>
#endif


IRecyclerBase::IRecyclerBase(io_context &ctx)
    : IOContext(ctx),
      Usage(-1),
      ShrinkTimer(nullptr),
      bExpanding(false) {

    static_assert(RECYCLER_SHRINK_RATE > RECYCLER_SHRINK_THRESHOLD);
}

IRecyclerBase::~IRecyclerBase() {
    if (ShrinkTimer != nullptr) {
        ShrinkTimer->cancel();
    }
}

std::shared_ptr<IRecycleInterface> IRecyclerBase::Acquire() {
    // Not Initialized
    if (Usage < 0)
        return nullptr;

    // Custom Deleter Of The Smart Pointer
    auto deleter = [weak = weak_from_this()](IRecycleInterface *obj) {
        if (const auto self = weak.lock()) {
            self->Recycle(obj);
        } else {
            delete obj;
        }
    };

    IRecycleInterface *elem = nullptr;

    // Pop The Front From The Queue If It Is Not Empty
    {
        std::unique_lock lock(Mutex);
        if (!InnerQueue.empty()) {
            auto ptr = std::move(InnerQueue.front());
            InnerQueue.pop();

            elem = ptr.release();

            SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Acquire Recyclable[{:p}] From Queue",
                __FUNCTION__, static_cast<void *>(this), static_cast<void *>(elem));
        }
    }

    // Not Available Element In The Queue, Create New One And Return The New
    if (elem == nullptr) {
        elem = Create();
        elem->OnCreate();

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Acquire New Recyclable[{:p}]",
            __FUNCTION__, static_cast<void *>(this), static_cast<void *>(elem));
    }

    elem->Initial();
    ++Usage;

    if (!bExpanding) {
        std::shared_lock lock(Mutex);

        // Check If It Needs To Expand
        if (static_cast<float>(Usage.load()) >= std::ceil(static_cast<float>(InnerQueue.size() + Usage.load()) * RECYCLER_EXPAND_THRESHOLD)) {
            bExpanding = true;
            co_spawn(IOContext, [weak = weak_from_this()]() -> awaitable<void> {
                if (const auto self = weak.lock()) {
                    self->Expand();
                }
                co_return;
            }, detached);
        }
    }

    return { elem, deleter };
}

size_t IRecyclerBase::GetUsage() const {
    if (Usage < 0)
        return 0;
    return Usage;
}

size_t IRecyclerBase::GetIdle() const {
    if (Usage < 0)
        return 0;

    std::shared_lock lock(Mutex);
    return InnerQueue.size();
}

size_t IRecyclerBase::GetCapacity() const {
    if (Usage < 0)
        return 0;

    std::shared_lock lock(Mutex);
    return InnerQueue.size() + Usage.load();
}

void IRecyclerBase::Shrink() {
    if (bExpanding) {
        ShrinkTimer.reset();
        return;
    }

    size_t num = 0;

    // Check If It Needs To Shrink
    {
        std::shared_lock lock(Mutex);

        // Recycler Total Capacity
        const size_t total = InnerQueue.size() + Usage.load();

        // Usage Less Than Shrink Threshold
        if (static_cast<float>(Usage.load()) < std::ceil(static_cast<float>(total) * RECYCLER_SHRINK_THRESHOLD)) {
            num = static_cast<size_t>(std::floor(static_cast<float>(total) * RECYCLER_SHRINK_RATE));

            const auto rest = total - num;
            if (rest <= 0) {
                ShrinkTimer.reset();
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

        std::unique_lock lock(Mutex);

        while (num-- > 0 && !InnerQueue.empty()) {
            const auto elem = std::move(InnerQueue.front());
            InnerQueue.pop();
        }

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] Shrink Finished",
            __FUNCTION__, static_cast<void *>(this));
    }

    ShrinkTimer.reset();
}

void IRecyclerBase::Recycle(IRecycleInterface *elem) {
    if (Usage < 0) {
        delete elem;
        return;
    }

    elem->Reset();
    --Usage;

    {
        std::unique_lock lock(Mutex);
        InnerQueue.emplace(elem);

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Recycle Recyclable[{:p}] To Queue",
            __FUNCTION__, static_cast<void *>(this), static_cast<void *>(elem));
    }

    if (ShrinkTimer != nullptr || bExpanding)
        return;

    // Do Shrink Later
    ShrinkTimer = make_shared<ASteadyTimer>(IOContext);

    co_spawn(IOContext, [weak = weak_from_this(), timer = ShrinkTimer]() mutable -> awaitable<void> {
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
    if (ShrinkTimer != nullptr) {
        ShrinkTimer->cancel();
    }

    size_t num = 0;
    std::vector<IRecycleInterface *> elems;

    // Calculate How Many New Elements Need To Be Created
    {
        std::shared_lock lock(Mutex);
        const size_t total = InnerQueue.size() + Usage.load();
        num = static_cast<size_t>(static_cast<float>(total) * RECYCLER_EXPAND_RATE);
    }

    while (num-- > 0) {
        auto *elem = Create();
        elem->OnCreate();

        elems.emplace_back(elem);
    }

    if (!elems.empty()) {
        std::unique_lock lock(Mutex);
        for (const auto &elem : elems) {
            InnerQueue.emplace(elem);
        }
    }

    bExpanding = false;
}

void IRecyclerBase::Initial(const size_t capacity) {
    if (Usage >= 0)
        return;

    for (size_t count = 0; count < capacity; count++) {
        auto *elem = Create();
        elem->OnCreate();

        InnerQueue.emplace(elem);
    }
    Usage = 0;
    SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Capacity[{}]", __FUNCTION__, static_cast<void *>(this), capacity);
}
