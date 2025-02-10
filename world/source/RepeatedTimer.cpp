#include "../include/RepeatedTimer.h"

#include <spdlog/spdlog.h>


URepeatedTimer::URepeatedTimer(asio::io_context &ctx)
    : ctx_(ctx),
      timer_(ctx),
      id_(),
      delay_(0),
      repeatRate_(0),
      is_repeat_(false),
      running_(false) {
}

URepeatedTimer::~URepeatedTimer() {
    Stop();
}

URepeatedTimer &URepeatedTimer::SetTimerID(const FUniqueID id) {
    id_ = id;
    return *this;
}

FUniqueID URepeatedTimer::GetTimerID() const {
    return id_;
}

URepeatedTimer & URepeatedTimer::SetDelay(std::chrono::duration<uint32_t> delay) {
    delay_ = delay;
    return *this;
}

URepeatedTimer &URepeatedTimer::SetRepeatRate(const std::chrono::duration<uint32_t> rate) {
    repeatRate_ = rate;
    return *this;
}

URepeatedTimer &URepeatedTimer::SetIfRepeat(const bool repeat) {
    is_repeat_ = repeat;
    return *this;
}

bool URepeatedTimer::IsRepeated() const {
    return is_repeat_;
}

bool URepeatedTimer::IsRunning() const {
    return running_;
}

bool URepeatedTimer::IsLooping() const {
    return is_repeat_ && running_;
}

URepeatedTimer &URepeatedTimer::SetCompleteCallback(const std::function<void(const FUniqueID &)> &func) {
    completeFunctor_ = func;
    return *this;
}

URepeatedTimer & URepeatedTimer::SetCompleteCallback(std::function<void(const FUniqueID &)> &&func) {
    completeFunctor_ = std::move(func);
    return *this;
}

URepeatedTimer &URepeatedTimer::Start() {
    if (repeatRate_.count() == 0)
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
                point += repeatRate_;

                co_await timer_.async_wait();

                if (running_)
                    std::invoke(functor_, point);
            } while (running_ && is_repeat_);

            running_ = false;

            if (completeFunctor_)
                std::invoke(completeFunctor_, id_);

        } catch (std::exception &e) {
            spdlog::warn("RepeatedTimer::start: {}", e.what());
        }
    }, detached);

    return *this;
}

URepeatedTimer &URepeatedTimer::Stop() {
    if (!running_)
        return *this;

    running_ = false;
    timer_.cancel();
    return *this;
}
