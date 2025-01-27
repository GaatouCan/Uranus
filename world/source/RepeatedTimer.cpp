#include "../include/RepeatedTimer.h"

#include <spdlog/spdlog.h>


URepeatedTimer::URepeatedTimer(asio::io_context &ctx)
    : mContext(ctx),
      mTimer(mContext),
      mTimerID(),
      mDelay(0),
      mRepeatRate(0),
      bRepeat(false),
      bRunning(false) {
}

URepeatedTimer::~URepeatedTimer() {
    Stop();
}

URepeatedTimer &URepeatedTimer::SetTimerID(const FUniqueID id) {
    mTimerID = id;
    return *this;
}

FUniqueID URepeatedTimer::GetTimerID() const {
    return mTimerID;
}

URepeatedTimer & URepeatedTimer::SetDelay(std::chrono::duration<uint32_t> delay) {
    mDelay = delay;
    return *this;
}

URepeatedTimer &URepeatedTimer::SetRepeatRate(const std::chrono::duration<uint32_t> rate) {
    mRepeatRate = rate;
    return *this;
}

URepeatedTimer &URepeatedTimer::SetIfRepeat(const bool repeat) {
    bRepeat = repeat;
    return *this;
}

bool URepeatedTimer::IsRepeated() const {
    return bRepeat;
}

bool URepeatedTimer::IsRunning() const {
    return bRunning;
}

bool URepeatedTimer::IsLooping() const {
    return bRepeat && bRunning;
}

URepeatedTimer &URepeatedTimer::SetCompleteCallback(const std::function<void(const FUniqueID &)> &func) {
    mCompleteFunctor = func;
    return *this;
}

URepeatedTimer & URepeatedTimer::SetCompleteCallback(std::function<void(const FUniqueID &)> &&func) {
    mCompleteFunctor = std::move(func);
    return *this;
}

URepeatedTimer &URepeatedTimer::Start() {
    if (mRepeatRate.count() == 0)
        return *this;

    if (bRunning)
        return *this;

    if (!mFunctor)
        return *this;

    co_spawn(mContext, [this]() mutable -> awaitable<void> {
        try {
            bRunning = true;
            auto point = NowTimePoint() + mDelay;

            do {
                mTimer.expires_at(point);
                point += mRepeatRate;

                co_await mTimer.async_wait();

                if (bRunning)
                    std::invoke(mFunctor, point);
            } while (bRunning && bRepeat);

            bRunning = false;

            if (mCompleteFunctor)
                std::invoke(mCompleteFunctor, mTimerID);

        } catch (std::exception &e) {
            spdlog::warn("RepeatedTimer::start: {}", e.what());
        }
    }, detached);

    return *this;
}

URepeatedTimer &URepeatedTimer::Stop() {
    if (!bRunning)
        return *this;

    bRunning = false;
    mTimer.cancel();
    return *this;
}
