#include "../../../include/system/timer/TimerSystem.h"
#include "../../../include/utils.h"

#include <ranges>


UTimerSystem::UTimerSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UTimerSystem::~UTimerSystem() {
    for (const auto timer: timerMap_ | std::views::values) {
        delete timer;
    }
}

void UTimerSystem::Init() {
}


URepeatedTimer *UTimerSystem::GetTimer(const FUniqueID &tid) {
    std::shared_lock lock(mutex_);
    if (const auto it = timerMap_.find(tid); it != timerMap_.end()) {
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
    std::unique_lock lock(mutex_);
    for (const auto timer: timerMap_ | std::views::values) {
        delete timer;
    }
}

std::optional<FUniqueID> UTimerSystem::EmplaceTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timerID = FUniqueID::RandomGenerate();

    {
        std::shared_lock lock(mutex_);
        while (timerMap_.contains(timerID)) {
            timerID = FUniqueID::RandomGenerate();
        }
    }

    {
        std::unique_lock lock(mutex_);
        timerMap_[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([self = this](const FUniqueID &id) mutable {
        if (self != nullptr) {
            self->RemoveTimer(id);
        }
    }).Start();

    return timerID;
}

bool UTimerSystem::RemoveTimer(const FUniqueID &tid) {
    std::unique_lock lock(mutex_);
    if (const auto it = timerMap_.find(tid); it != timerMap_.end()) {
        const auto timer = it->second;
        timerMap_.erase(it);

        if (timer != nullptr) {
            delete timer;
            return true;
        }
    }
    return false;
}
