#include "../../../include/system/timer/TimerSystem.h"
#include "../../../include/utils.h"

#include <ranges>


UTimerSystem::UTimerSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UTimerSystem::~UTimerSystem() {
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

void UTimerSystem::Init() {
}


URepeatedTimer *UTimerSystem::GetTimer(const FUniqueID &tid) {
    std::shared_lock lock(mSharedMutex);
    if (const auto it = mTimerMap.find(tid); it != mTimerMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool UTimerSystem::StopTimer(const FUniqueID &tid) {
    const auto timer = GetTimer(tid);
    if (timer == nullptr)
        return false;

    timer->Stop();
    return true;
}

void UTimerSystem::CleanAllTimers() {
    std::scoped_lock lock(mMutex);
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

std::optional<FUniqueID> UTimerSystem::EmplaceTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timerID = FUniqueID::RandomGenerate();

    {
        std::shared_lock lock(mSharedMutex);
        while (mTimerMap.contains(timerID)) {
            timerID = FUniqueID::RandomGenerate();
        }
    }

    {
        std::scoped_lock lock(mMutex);
        mTimerMap[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([self = this](const FUniqueID &id) mutable {
        if (self != nullptr) {
            self->RemoveTimer(id);
        }
    }).Start();

    return timerID;
}

bool UTimerSystem::RemoveTimer(const FUniqueID &tid) {
    std::scoped_lock lock(mMutex);
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
