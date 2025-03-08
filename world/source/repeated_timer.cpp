#include "../include/repeated_timer.h"

#include <spdlog/spdlog.h>


URepeatedTimer::URepeatedTimer(asio::io_context &ctx)
    : context_(ctx),
      timer_(ctx),
      id_(),
      delay_(0),
      repeatRate_(0),
      repeated_(false),
      running_(false) {
}

URepeatedTimer::~URepeatedTimer() {
    stop();
}

URepeatedTimer &URepeatedTimer::setTimerID(const FUniqueID id) {
    id_ = id;
    return *this;
}

FUniqueID URepeatedTimer::getTimerID() const {
    return id_;
}

URepeatedTimer & URepeatedTimer::setDelay(std::chrono::duration<uint32_t> delay) {
    delay_ = delay;
    return *this;
}

URepeatedTimer &URepeatedTimer::setRepeatRate(const std::chrono::duration<uint32_t> rate) {
    repeatRate_ = rate;
    return *this;
}

URepeatedTimer &URepeatedTimer::setIfRepeat(const bool repeat) {
    repeated_ = repeat;
    return *this;
}

bool URepeatedTimer::repeated() const {
    return repeated_;
}

bool URepeatedTimer::running() const {
    return running_;
}

bool URepeatedTimer::looping() const {
    return repeated_ && running_;
}

URepeatedTimer &URepeatedTimer::setCompleteCallback(const std::function<void(const FUniqueID &)> &func) {
    completeFunctor_ = func;
    return *this;
}

URepeatedTimer & URepeatedTimer::setCompleteCallback(std::function<void(const FUniqueID &)> &&func) {
    completeFunctor_ = std::move(func);
    return *this;
}

URepeatedTimer &URepeatedTimer::start() {
    if (repeatRate_.count() == 0)
        return *this;

    if (running_)
        return *this;

    if (!functor_)
        return *this;

    co_spawn(context_, [this]() mutable -> awaitable<void> {
        try {
            running_ = true;
            auto point = NowTimePoint() + delay_;

            do {
                timer_.expires_at(point);
                point += repeatRate_;

                co_await timer_.async_wait();

                if (running_)
                    functor_(point);
            } while (running_ && repeated_);

            running_ = false;

            if (completeFunctor_)
                completeFunctor_(id_);

        } catch (std::exception &e) {
            spdlog::warn("RepeatedTimer::start: {}", e.what());
        }
    }, detached);

    return *this;
}

URepeatedTimer &URepeatedTimer::stop() {
    if (!running_)
        return *this;

    running_ = false;
    timer_.cancel();
    return *this;
}
