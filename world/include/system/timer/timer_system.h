#pragma once

#include "../../sub_system.h"
#include "../../repeated_timer.h"

#include <map>
#include <shared_mutex>


class BASE_API UTimerSystem final : public ISubSystem {

    std::map<FUniqueID, URepeatedTimer *> timer_map_;
    mutable std::shared_mutex mtx_;

public:
    explicit UTimerSystem(UGameWorld *world);
    ~UTimerSystem() override;

    GET_SYSTEM_NAME(TimerSystem)

    void Init() override;

    template<typename Functor, typename... Args>
    std::optional<FUniqueID> SetTimer(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Args &&... args)
    {
        const auto timer = new URepeatedTimer(GetIOContext());
        timer->SetDelay(delay)
            .SetRepeatRate(rate)
            .SetIfRepeat(repeat)
            .SetFunctor(std::forward<Functor>(func), std::forward<Args>(args)...);

        const auto tid = EmplaceTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    template<typename Functor, typename Object, typename... Args>
    std::optional<FUniqueID> SetTimerTo (
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Object *obj, Args &&... args)
    {
        const auto timer = new URepeatedTimer(GetIOContext());
        timer->SetDelay(delay)
            .SetRepeatRate(rate)
            .SetIfRepeat(repeat)
            .SetMemberFunctor(std::forward<Functor>(func), obj, std::forward<Args>(args)...);

        const auto tid = EmplaceTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    URepeatedTimer *GetTimer(const FUniqueID &tid);
    bool StopTimer(const FUniqueID &tid);

    void CleanAllTimers();
private:
    std::optional<FUniqueID> EmplaceTimer(URepeatedTimer *timer);
    bool RemoveTimer(const FUniqueID &tid);
};

