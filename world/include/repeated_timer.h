#pragma once

#include "unique_id.h"
#include "utils.h"

using TimerFunctor = std::function<void(TimePoint)>;

class BASE_API RepeatedTimer final {

    asio::io_context &mContext;
    SystemTimer mTimer;

    UniqueID mID;
    TimerFunctor mFunctor;

    std::function<void(const UniqueID &)> mCompleteFunctor;

    std::chrono::duration<uint32_t> mDelay;
    std::chrono::duration<uint32_t> mRepeatRate;
    bool bRepeat;

    std::atomic<bool> bRunning;

public:
    RepeatedTimer() = delete;

    explicit RepeatedTimer(asio::io_context &ctx);
    ~RepeatedTimer();

    DISABLE_COPY_MOVE(RepeatedTimer)

    RepeatedTimer &SetTimerID(UniqueID id);
    [[nodiscard]] UniqueID GetTimerID() const;

    RepeatedTimer &SetDelay(std::chrono::duration<uint32_t> delay);

    RepeatedTimer &SetRepeatRate(std::chrono::duration<uint32_t> rate);
    RepeatedTimer &SetIfRepeat(bool repeat);

    [[nodiscard]] bool IsRepeated() const;

    [[nodiscard]] bool IsRunning() const;
    [[nodiscard]] bool IsLooping() const;

    template<typename Functor, typename... Args>
    RepeatedTimer &SetFunctor(Functor &&func, Args &&... args) {
        mFunctor = [func = std::forward<Functor>(func), ...args = std::forward<Args>(args)](TimePoint point) mutable {
            std::invoke(func, point, args...);
        };
        return *this;
    }

    template<typename Functor, typename Object, typename... Args>
    RepeatedTimer &SetMemberFunctor(Functor &&func, Object *obj, Args &&... args) {
        mFunctor = [func = std::forward<Functor>(func), obj, ...args = std::forward<Args>(args)](TimePoint point) mutable {
            std::invoke(func, obj, point, args...);
        };
        return *this;
    }

    RepeatedTimer &SetCompleteCallback(const std::function<void(const UniqueID &)> &func);
    RepeatedTimer &SetCompleteCallback(std::function<void(const UniqueID &)> &&func);

    RepeatedTimer &Start();
    RepeatedTimer &Stop();
};
