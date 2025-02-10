#pragma once

#include "UniqueID.h"
#include "utils.h"

using ATimerFunctor = std::function<void(ATimePoint)>;

class BASE_API URepeatedTimer final {

    asio::io_context &ctx_;
    ASystemTimer timer_;

    FUniqueID id_;
    ATimerFunctor functor_;

    std::function<void(const FUniqueID &)> completeFunctor_;

    std::chrono::duration<uint32_t> delay_;
    std::chrono::duration<uint32_t> repeatRate_;
    bool is_repeat_;

    std::atomic<bool> running_;

public:
    URepeatedTimer() = delete;

    explicit URepeatedTimer(asio::io_context &ctx);
    ~URepeatedTimer();

    DISABLE_COPY_MOVE(URepeatedTimer)

    URepeatedTimer &SetTimerID(FUniqueID id);
    [[nodiscard]] FUniqueID GetTimerID() const;

    URepeatedTimer &SetDelay(std::chrono::duration<uint32_t> delay);

    URepeatedTimer &SetRepeatRate(std::chrono::duration<uint32_t> rate);
    URepeatedTimer &SetIfRepeat(bool repeat);

    [[nodiscard]] bool IsRepeated() const;

    [[nodiscard]] bool IsRunning() const;
    [[nodiscard]] bool IsLooping() const;

    template<typename Functor, typename... Args>
    URepeatedTimer &SetFunctor(Functor &&func, Args &&... args) {
        functor_ = [func = std::forward<Functor>(func), ...args = std::forward<Args>(args)](ATimePoint point) mutable {
            std::invoke(func, point, args...);
        };
        return *this;
    }

    template<typename Functor, typename Object, typename... Args>
    URepeatedTimer &SetMemberFunctor(Functor &&func, Object *obj, Args &&... args) {
        functor_ = [func = std::forward<Functor>(func), obj, ...args = std::forward<Args>(args)](ATimePoint point) mutable {
            std::invoke(func, obj, point, args...);
        };
        return *this;
    }

    URepeatedTimer &SetCompleteCallback(const std::function<void(const FUniqueID &)> &func);
    URepeatedTimer &SetCompleteCallback(std::function<void(const FUniqueID &)> &&func);

    URepeatedTimer &Start();
    URepeatedTimer &Stop();
};
