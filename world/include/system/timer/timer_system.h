#pragma once

#include "../../sub_system.h"
#include "../../repeated_timer.h"

#include <map>
#include <shared_mutex>


class BASE_API TimerSystem final : public ISubSystem {

    std::map<UniqueID, RepeatedTimer *> timer_map_;
    mutable std::shared_mutex mtx_;

public:
    explicit TimerSystem(GameWorld *world);
    ~TimerSystem() override;

    GET_SYSTEM_NAME(TimerSystem)

    void Init() override;

    template<typename Functor, typename... Args>
    std::optional<UniqueID> SetTimer(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Args &&... args)
    {
        const auto timer = new RepeatedTimer(GetIOContext());
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
    std::optional<UniqueID> SetTimerTo (
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Object *obj, Args &&... args)
    {
        const auto timer = new RepeatedTimer(GetIOContext());
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

    RepeatedTimer *GetTimer(const UniqueID &tid);
    bool StopTimer(const UniqueID &tid);

    void CleanAllTimers();
private:
    std::optional<UniqueID> EmplaceTimer(RepeatedTimer *timer);
    bool RemoveTimer(const UniqueID &tid);
};

