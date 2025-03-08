#include "../../../include/system/timer/timer_system.h"
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

void UTimerSystem::init() {
}


URepeatedTimer *UTimerSystem::getTimer(const FUniqueID &tid) {
    std::shared_lock lock(mutex_);
    if (const auto it = timerMap_.find(tid); it != timerMap_.end()) {
        return it->second;
    }
    return nullptr;
}

bool UTimerSystem::stopTimer(const FUniqueID &tid) {
    const auto timer = getTimer(tid);
    if (timer == nullptr)
        return false;

    timer->stop();
    return true;
}

void UTimerSystem::cleanAllTimers() {
    std::unique_lock lock(mutex_);
    for (const auto timer: timerMap_ | std::views::values) {
        delete timer;
    }
}

std::optional<FUniqueID> UTimerSystem::emplaceTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timerID = FUniqueID::randomGenerate();

    {
        std::shared_lock lock(mutex_);
        while (timerMap_.contains(timerID)) {
            timerID = FUniqueID::randomGenerate();
        }
    }

    {
        std::unique_lock lock(mutex_);
        timerMap_[timerID] = timer;
    }

    timer->setTimerID(timerID).setCompleteCallback([self = this](const FUniqueID &id) mutable {
        if (self != nullptr) {
            self->removeTimer(id);
        }
    }).start();

    return timerID;
}

bool UTimerSystem::removeTimer(const FUniqueID &tid) {
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
