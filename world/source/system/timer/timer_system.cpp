#include "../../../include/system/timer/timer_system.h"
#include "../../../include/utils.h"

#include <ranges>


TimerSystem::TimerSystem(GameWorld *world)
    : ISubSystem(world) {
}

TimerSystem::~TimerSystem() {
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

void TimerSystem::Init() {
}


RepeatedTimer *TimerSystem::GetTimer(const UniqueID &tid) {
    std::shared_lock lock(mMutex);
    if (const auto it = mTimerMap.find(tid); it != mTimerMap.end()) {
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
    std::unique_lock lock(mMutex);
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

std::optional<UniqueID> TimerSystem::EmplaceTimer(RepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    UniqueID timerID = UniqueID::RandomGenerate();

    {
        std::shared_lock lock(mMutex);
        while (mTimerMap.contains(timerID)) {
            timerID = UniqueID::RandomGenerate();
        }
    }

    {
        std::unique_lock lock(mMutex);
        mTimerMap[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([self = this](const UniqueID &id) mutable {
        if (self != nullptr) {
            self->RemoveTimer(id);
        }
    }).Start();

    return timerID;
}

bool TimerSystem::RemoveTimer(const UniqueID &tid) {
    std::unique_lock lock(mMutex);
    if (const auto it = mTimerMap.find(tid); it != mTimerMap.end()) {
        const auto timer = it->second;
        mTimerMap.erase(it);

        if (timer != nullptr) {
            delete timer;
            return true;
        }
    }
    return false;
}
