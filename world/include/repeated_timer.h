#pragma once

#include "unique_id.h"
#include "utils.h"

using ATimerFunctor = std::function<void(ATimePoint)>;

class BASE_API URepeatedTimer final {

    asio::io_context &context_;
    ASystemTimer timer_;

    FUniqueID id_;
    ATimerFunctor functor_;

    std::function<void(const FUniqueID &)> completeFunctor_;

    std::chrono::duration<uint32_t> delay_;
    std::chrono::duration<uint32_t> repeatRate_;
    bool repeated_;

    std::atomic<bool> running_;

public:
    URepeatedTimer() = delete;

    explicit URepeatedTimer(asio::io_context &ctx);
    ~URepeatedTimer();

    DISABLE_COPY_MOVE(URepeatedTimer)

    URepeatedTimer &setTimerID(FUniqueID id);
    [[nodiscard]] FUniqueID getTimerID() const;

    URepeatedTimer &setDelay(std::chrono::duration<uint32_t> delay);

    URepeatedTimer &setRepeatRate(std::chrono::duration<uint32_t> rate);
    URepeatedTimer &setIfRepeat(bool repeat);

    [[nodiscard]] bool repeated() const;

    [[nodiscard]] bool running() const;
    [[nodiscard]] bool looping() const;

    template<typename Functor, typename... Args>
    URepeatedTimer &setFunctor(Functor &&func, Args &&... args) {
        functor_ = [func = std::forward<Functor>(func), ...args = std::forward<Args>(args)](ATimePoint point) mutable {
            std::invoke(func, point, args...);
        };
        return *this;
    }

    template<typename Functor, typename Object, typename... Args>
    URepeatedTimer &setMemberFunctor(Functor &&func, Object *obj, Args &&... args) {
        functor_ = [func = std::forward<Functor>(func), obj, ...args = std::forward<Args>(args)](ATimePoint point) mutable {
            std::invoke(func, obj, point, args...);
        };
        return *this;
    }

    URepeatedTimer &setCompleteCallback(const std::function<void(const FUniqueID &)> &func);
    URepeatedTimer &setCompleteCallback(std::function<void(const FUniqueID &)> &&func);

    URepeatedTimer &start();
    URepeatedTimer &stop();
};
