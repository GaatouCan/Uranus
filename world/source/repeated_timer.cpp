#include "../include/repeated_timer.h"

#include <spdlog/spdlog.h>


RepeatedTimer::RepeatedTimer(asio::io_context &ctx)
    : mContext(ctx),
      mTimer(ctx),
      mID(),
      mDelay(0),
      mRepeatRate(0),
      bRepeat(false),
      bRunning(false) {
}

RepeatedTimer::~RepeatedTimer() {
    Stop();
}

RepeatedTimer &RepeatedTimer::SetTimerID(const UniqueID id) {
    mID = id;
    return *this;
}

UniqueID RepeatedTimer::GetTimerID() const {
    return mID;
}

RepeatedTimer & RepeatedTimer::SetDelay(std::chrono::duration<uint32_t> delay) {
    mDelay = delay;
    return *this;
}

RepeatedTimer &RepeatedTimer::SetRepeatRate(const std::chrono::duration<uint32_t> rate) {
    mRepeatRate = rate;
    return *this;
}

RepeatedTimer &RepeatedTimer::SetIfRepeat(const bool repeat) {
    bRepeat = repeat;
    return *this;
}

bool RepeatedTimer::IsRepeated() const {
    return bRepeat;
}

bool RepeatedTimer::IsRunning() const {
    return bRunning;
}

bool RepeatedTimer::IsLooping() const {
    return bRepeat && bRunning;
}

RepeatedTimer &RepeatedTimer::SetCompleteCallback(const std::function<void(const UniqueID &)> &func) {
    mCompleteFunctor = func;
    return *this;
}

RepeatedTimer & RepeatedTimer::SetCompleteCallback(std::function<void(const UniqueID &)> &&func) {
    mCompleteFunctor = std::move(func);
    return *this;
}

RepeatedTimer &RepeatedTimer::Start() {
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
                    mFunctor(point);
            } while (bRunning && bRepeat);

            bRunning = false;

            if (mCompleteFunctor)
                mCompleteFunctor(mID);

        } catch (std::exception &e) {
            spdlog::warn("RepeatedTimer::start: {}", e.what());
        }
    }, detached);

    return *this;
}

RepeatedTimer &RepeatedTimer::Stop() {
    if (!bRunning)
        return *this;

    bRunning = false;
    mTimer.cancel();
    return *this;
}
