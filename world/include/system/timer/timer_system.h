#pragma once

#include "../../sub_system.h"
#include "../../repeated_timer.h"

#include <map>
#include <shared_mutex>


class BASE_API UTimerSystem final : public ISubSystem {

    std::map<FUniqueID, URepeatedTimer *> timerMap_;
    mutable std::shared_mutex mutex_;

public:
    explicit UTimerSystem(UGameWorld *world);
    ~UTimerSystem() override;

    GET_SYSTEM_NAME(TimerSystem)

    void init() override;

    template<typename Functor, typename... Args>
    std::optional<FUniqueID> setTimer(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Args &&... args)
    {
        const auto timer = new URepeatedTimer(getIOContext());
        timer->setDelay(delay)
            .setRepeatRate(rate)
            .setIfRepeat(repeat)
            .setFunctor(std::forward<Functor>(func), std::forward<Args>(args)...);

        const auto tid = emplaceTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    template<typename Functor, typename Object, typename... Args>
    std::optional<FUniqueID> setTimerTo (
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Object *obj, Args &&... args)
    {
        const auto timer = new URepeatedTimer(getIOContext());
        timer->setDelay(delay)
            .setRepeatRate(rate)
            .setIfRepeat(repeat)
            .setMemberFunctor(std::forward<Functor>(func), obj, std::forward<Args>(args)...);

        const auto tid = emplaceTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    URepeatedTimer *getTimer(const FUniqueID &tid);
    bool stopTimer(const FUniqueID &tid);

    void cleanAllTimers();
private:
    std::optional<FUniqueID> emplaceTimer(URepeatedTimer *timer);
    bool removeTimer(const FUniqueID &tid);
};

