#include "../include/repeated_timer.h"

#include <spdlog/spdlog.h>


RepeatedTimer::RepeatedTimer(asio::io_context &ctx)
    : ctx_(ctx),
      timer_(ctx),
      id_(),
      delay_(0),
      repeat_rate_(0),
      repeat_(false),
      running_(false) {
}

RepeatedTimer::~RepeatedTimer() {
    Stop();
}

RepeatedTimer &RepeatedTimer::SetTimerID(const UniqueID id) {
    id_ = id;
    return *this;
}

UniqueID RepeatedTimer::GetTimerID() const {
    return id_;
}

RepeatedTimer & RepeatedTimer::SetDelay(std::chrono::duration<uint32_t> delay) {
    delay_ = delay;
    return *this;
}

RepeatedTimer &RepeatedTimer::SetRepeatRate(const std::chrono::duration<uint32_t> rate) {
    repeat_rate_ = rate;
    return *this;
}

RepeatedTimer &RepeatedTimer::SetIfRepeat(const bool repeat) {
    repeat_ = repeat;
    return *this;
}

bool RepeatedTimer::IsRepeated() const {
    return repeat_;
}

bool RepeatedTimer::IsRunning() const {
    return running_;
}

bool RepeatedTimer::IsLooping() const {
    return repeat_ && running_;
}

RepeatedTimer &RepeatedTimer::SetCompleteCallback(const std::function<void(const UniqueID &)> &func) {
    complete_functor_ = func;
    return *this;
}

RepeatedTimer & RepeatedTimer::SetCompleteCallback(std::function<void(const UniqueID &)> &&func) {
    complete_functor_ = std::move(func);
    return *this;
}

RepeatedTimer &RepeatedTimer::Start() {
    if (repeat_rate_.count() == 0)
        return *this;

    if (running_)
        return *this;

    if (!functor_)
        return *this;

    co_spawn(ctx_, [this]() mutable -> awaitable<void> {
        try {
            running_ = true;
            auto point = NowTimePoint() + delay_;

            do {
                timer_.expires_at(point);
                point += repeat_rate_;

                co_await timer_.async_wait();

                if (running_)
                    functor_(point);
            } while (running_ && repeat_);

            running_ = false;

            if (complete_functor_)
                complete_functor_(id_);

        } catch (std::exception &e) {
            spdlog::warn("RepeatedTimer::start: {}", e.what());
        }
    }, detached);

    return *this;
}

RepeatedTimer &RepeatedTimer::Stop() {
    if (!running_)
        return *this;

    running_ = false;
    timer_.cancel();
    return *this;
}
