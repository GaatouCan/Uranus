#include "Recycler.h"

#include <spdlog/spdlog.h>
#ifdef __linux__
#include <mutex>
#endif


IRecyclerBase::IRecyclerBase(io_context &ctx)
    : ctx_(ctx),
      usage_(-1),
      timer_(nullptr),
      bExpanding(false) {

    static_assert(RECYCLER_SHRINK_RATE > RECYCLER_SHRINK_THRESHOLD);
}

IRecyclerBase::~IRecyclerBase() {
    if (timer_ != nullptr) {
        timer_->cancel();
    }
}

std::shared_ptr<IRecycleInterface> IRecyclerBase::Acquire() {
    // Not Initialized
    if (usage_ < 0)
        return nullptr;

    // Custom Deleter Of The Smart Pointer
    auto deleter = [weak = weak_from_this()](IRecycleInterface *elem) {
        if (const auto self = weak.lock()) {
            self->Recycle(elem);
        } else {
            delete elem;
        }
    };

    IRecycleInterface *elem = nullptr;

    // Pop The Front From The Queue If It Is Not Empty
    {
        std::unique_lock lock(mutex_);
        if (!queue_.empty()) {
            auto ptr = std::move(queue_.front());
            queue_.pop();

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
    ++usage_;

    if (!bExpanding) {
        std::shared_lock lock(mutex_);

        // Check If It Needs To Expand
        if (static_cast<float>(usage_.load()) >= std::ceil(static_cast<float>(queue_.size() + usage_.load()) * RECYCLER_EXPAND_THRESHOLD)) {
            bExpanding = true;
            co_spawn(ctx_, [weak = weak_from_this()]() -> awaitable<void> {
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
    if (usage_ < 0)
        return 0;
    return usage_;
}

size_t IRecyclerBase::GetIdle() const {
    if (usage_ < 0)
        return 0;

    std::shared_lock lock(mutex_);
    return queue_.size();
}

size_t IRecyclerBase::GetCapacity() const {
    if (usage_ < 0)
        return 0;

    std::shared_lock lock(mutex_);
    return queue_.size() + usage_.load();
}

void IRecyclerBase::Shrink() {
    if (bExpanding) {
        timer_.reset();
        return;
    }

    size_t num = 0;

    // Check If It Needs To Shrink
    {
        std::shared_lock lock(mutex_);

        // Recycler Total Capacity
        const size_t total = queue_.size() + usage_.load();

        // Usage Less Than Shrink Threshold
        if (static_cast<float>(usage_.load()) < std::ceil(static_cast<float>(total) * RECYCLER_SHRINK_THRESHOLD)) {
            num = static_cast<size_t>(std::floor(static_cast<float>(total) * RECYCLER_SHRINK_RATE));

            const auto rest = total - num;
            if (rest <= 0) {
                timer_.reset();
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

        std::unique_lock lock(mutex_);

        while (num-- > 0 && !queue_.empty()) {
            const auto elem = std::move(queue_.front());
            queue_.pop();
        }

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] Shrink Finished",
            __FUNCTION__, static_cast<void *>(this));
    }

    timer_.reset();
}

void IRecyclerBase::Recycle(IRecycleInterface *elem) {
    if (usage_ < 0) {
        delete elem;
        return;
    }

    elem->Reset();
    --usage_;

    {
        std::unique_lock lock(mutex_);
        queue_.emplace(elem);

        SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Recycle Recyclable[{:p}] To Queue",
            __FUNCTION__, static_cast<void *>(this), static_cast<void *>(elem));
    }

    if (timer_ != nullptr || bExpanding)
        return;

    // Do Shrink Later
    timer_ = make_shared<ASteadyTimer>(ctx_);

    co_spawn(ctx_, [weak = weak_from_this(), timer = timer_]() mutable -> awaitable<void> {
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
    if (timer_ != nullptr) {
        timer_->cancel();
    }

    size_t num = 0;
    std::vector<IRecycleInterface *> elems;

    // Calculate How Many New Elements Need To Be Created
    {
        std::shared_lock lock(mutex_);
        const size_t total = queue_.size() + usage_.load();
        num = static_cast<size_t>(static_cast<float>(total) * RECYCLER_EXPAND_RATE);
    }

    while (num-- > 0) {
        auto *elem = Create();
        elem->OnCreate();

        elems.emplace_back(elem);
    }

    if (!elems.empty()) {
        std::unique_lock lock(mutex_);
        for (const auto &elem : elems) {
            queue_.emplace(elem);
        }
    }

    bExpanding = false;
}

void IRecyclerBase::Initial(const size_t capacity) {
    if (usage_ >= 0)
        return;

    for (size_t count = 0; count < capacity; count++) {
        auto *elem = Create();
        elem->OnCreate();

        queue_.emplace(elem);
    }
    usage_ = 0;
    SPDLOG_TRACE("{:<20} - Recycler[{:p}] - Capacity[{}]", __FUNCTION__, static_cast<void *>(this), capacity);
}
