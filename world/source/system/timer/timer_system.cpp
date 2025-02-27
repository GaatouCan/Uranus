#include "../../../include/system/timer/timer_system.h"
#include "../../../include/utils.h"

#include <ranges>


TimerSystem::TimerSystem(GameWorld *world)
    : ISubSystem(world) {
}

TimerSystem::~TimerSystem() {
    for (const auto timer: timer_map_ | std::views::values) {
        delete timer;
    }
}

void TimerSystem::Init() {
}


RepeatedTimer *TimerSystem::GetTimer(const UniqueID &tid) {
    std::shared_lock lock(mtx_);
    if (const auto it = timer_map_.find(tid); it != timer_map_.end()) {
        return it->second;
    }
    return nullptr;
}

bool TimerSystem::StopTimer(const UniqueID &tid) {
    const auto timer = GetTimer(tid);
    if (timer == nullptr)
        return false;

    timer->Stop();
    return true;
}

void TimerSystem::CleanAllTimers() {
    std::unique_lock lock(mtx_);
    for (const auto timer: timer_map_ | std::views::values) {
        delete timer;
    }
}

std::optional<UniqueID> TimerSystem::EmplaceTimer(RepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    UniqueID timerID = UniqueID::RandomGenerate();

    {
        std::shared_lock lock(mtx_);
        while (timer_map_.contains(timerID)) {
            timerID = UniqueID::RandomGenerate();
        }
    }

    {
        std::unique_lock lock(mtx_);
        timer_map_[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([self = this](const UniqueID &id) mutable {
        if (self != nullptr) {
            self->RemoveTimer(id);
        }
    }).Start();

    return timerID;
}

bool TimerSystem::RemoveTimer(const UniqueID &tid) {
    std::unique_lock lock(mtx_);
    if (const auto it = timer_map_.find(tid); it != timer_map_.end()) {
        const auto timer = it->second;
        timer_map_.erase(it);

        if (timer != nullptr) {
            delete timer;
            return true;
        }
    }
    return false;
}
