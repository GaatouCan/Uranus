#include "../../../include/system/timer/timer_system.h"
#include "../../../include/utils.h"

#include <ranges>


UTimerSystem::UTimerSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UTimerSystem::~UTimerSystem() {
    for (const auto timer: timer_map_ | std::views::values) {
        delete timer;
    }
}

void UTimerSystem::Init() {
}


URepeatedTimer *UTimerSystem::GetTimer(const FUniqueID &tid) {
    std::shared_lock lock(mtx_);
    if (const auto it = timer_map_.find(tid); it != timer_map_.end()) {
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
    std::unique_lock lock(mtx_);
    for (const auto timer: timer_map_ | std::views::values) {
        delete timer;
    }
}

std::optional<FUniqueID> UTimerSystem::EmplaceTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timerID = FUniqueID::randomGenerate();

    {
        std::shared_lock lock(mtx_);
        while (timer_map_.contains(timerID)) {
            timerID = FUniqueID::randomGenerate();
        }
    }

    {
        std::unique_lock lock(mtx_);
        timer_map_[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([self = this](const FUniqueID &id) mutable {
        if (self != nullptr) {
            self->RemoveTimer(id);
        }
    }).Start();

    return timerID;
}

bool UTimerSystem::RemoveTimer(const FUniqueID &tid) {
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
