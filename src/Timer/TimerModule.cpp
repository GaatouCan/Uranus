#include "TimerModule.h"
#include "Server.h"
#include "Gateway/Gateway.h"
#include "Gateway/PlayerAgent.h"
#include "Service/ServiceModule.h"

#include <spdlog/spdlog.h>


UTimerModule::UTimerModule() {
}

UTimerModule::~UTimerModule() {
    Stop();
}

FTimerHandle UTimerModule::SetSteadyTimer(const int32_t sid, const int64_t pid, const ATimerTask &task, int delay, int rate) {
    const auto id = allocator_.AllocateConcurrent();
    if (id < 0)
        return { -1, true };

    {
        std::unique_lock lock(timerMutex_);

        if (steadyTimerMap_.contains(id))
            return { -2, true };

        FSteadyTimerNode node { sid, pid, make_shared<ASteadyTimer>(GetServer()->GetIOContext()) };
        steadyTimerMap_.emplace(id, std::move(node));

        if (sid > 0)
            serviceToSteadyTimer_[sid].emplace(id);
        else
            playerToSteadyTimer_[pid].emplace(id);
    }

    co_spawn(GetServer()->GetIOContext(), [this, timer = steadyTimerMap_[id].timer, id, sid, pid, delay, rate, task]() -> awaitable<void> {
        try {
            auto point = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay * 100);

            do {
                timer->expires_at(point);
                if (rate > 0) {
                    point += std::chrono::milliseconds(rate * 100);
                }

                if (auto [ec] = co_await timer->async_wait(); ec) {
                    SPDLOG_DEBUG("{:<20} - Timer[{}] Canceled", "UTimerModule::SetTimer()", id);
                    break;
                }

                if (sid > 0) {
                    if (const auto *service = GetServer()->GetModule<UServiceModule>()) {
                        if (const auto context = service->FindService(sid)) {
                            context->PushTask(task);
                        }
                    }
                } else {
                    if (const auto *gateway = GetServer()->GetModule<UGateway>()) {
                        if (const auto player = gateway->FindPlayerAgent(pid)) {
                            player->PushTask(task);
                        }
                    }
                }

            } while (rate > 0 && state_ == EModuleState::RUNNING);
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{:<20} - Exception: {}", "UTimerModule::SetTimer", e.what());
        }

        RemoveSteadyTimer(id);
        allocator_.RecycleConcurrent(id);
    }, detached);

    return { id, true };
}

FTimerHandle UTimerModule::SetSystemTimer(int32_t sid, int64_t pid, const ATimerTask &task, int delay, int rate) {
    const auto id = allocator_.AllocateConcurrent();
    if (id < 0)
        return { -1, false };

    {
        std::unique_lock lock(timerMutex_);

        if (steadyTimerMap_.contains(id))
            return { -2,  true };

        FSystemTimerNode node { sid, pid, make_shared<ASystemTimer>(GetServer()->GetIOContext()) };
        systemTimerMap_.emplace(id, std::move(node));

        if (sid > 0)
            serviceToSystemTimer_[sid].emplace(id);
        else
            playerToSystemTimer_[pid].emplace(id);
    }

    co_spawn(GetServer()->GetIOContext(), [this, timer = systemTimerMap_[id].timer, id, sid, pid, delay, rate, task]() -> awaitable<void> {
        try {
            auto point = std::chrono::system_clock::now() + std::chrono::milliseconds(delay * 100);

            do {
                timer->expires_at(point);
                if (rate > 0) {
                    point += std::chrono::milliseconds(rate * 100);
                }

                if (auto [ec] = co_await timer->async_wait(); ec) {
                    SPDLOG_DEBUG("{:<20} - Timer[{}] Canceled", "UTimerModule::SetTimer()", id);
                    break;
                }

                if (sid > 0) {
                    if (const auto *service = GetServer()->GetModule<UServiceModule>()) {
                        if (const auto context = service->FindService(sid)) {
                            context->PushTask(task);
                        }
                    }
                } else {
                    if (const auto *gateway = GetServer()->GetModule<UGateway>()) {
                        if (const auto player = gateway->FindPlayerAgent(pid)) {
                            player->PushTask(task);
                        }
                    }
                }

            } while (rate > 0 && state_ == EModuleState::RUNNING);
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{:<20} - Exception: {}", "UTimerModule::SetTimer", e.what());
        }

        RemoveSystemTimer(id);
        allocator_.RecycleConcurrent(id);
    }, detached);

    return { id, false };
}

void UTimerModule::CancelTimer(const FTimerHandle &handle) {
    if (state_ != EModuleState::RUNNING)
        return;

    std::unique_lock lock(timerMutex_);
    if (handle.bSteady) {
        const auto iter = steadyTimerMap_.find(handle.id);
        if (iter == steadyTimerMap_.end())
            return;

        iter->second.timer->cancel();

        if (iter->second.sid > 0) {
            if (const auto it = serviceToSteadyTimer_.find(iter->second.sid); it != serviceToSteadyTimer_.end())
                it->second.erase(handle.id);
        } else {
            if (const auto it = playerToSteadyTimer_.find(iter->second.pid); it != playerToSteadyTimer_.end())
                it->second.erase(handle.id);
        }

        steadyTimerMap_.erase(iter);
    } else {
        const auto iter = systemTimerMap_.find(handle.id);
        if (iter == systemTimerMap_.end())
            return;

        iter->second.timer->cancel();

        if (iter->second.sid > 0) {
            if (const auto it = serviceToSystemTimer_.find(iter->second.sid); it != serviceToSystemTimer_.end())
                it->second.erase(handle.id);
        } else {
            if (const auto it = playerToSystemTimer_.find(iter->second.pid); it != playerToSystemTimer_.end())
                it->second.erase(handle.id);
        }

        systemTimerMap_.erase(iter);
    }
}

void UTimerModule::CancelServiceTimer(const int32_t sid) {
    if (state_ != EModuleState::RUNNING)
        return;

    std::unique_lock lock(timerMutex_);

    // Clean Steady Timers
    const auto steadyIter = serviceToSteadyTimer_.find(sid);
    if (steadyIter == serviceToSteadyTimer_.end())
        return;

    for (const auto tid : steadyIter->second) {
        const auto timerIter = steadyTimerMap_.find(tid);
        if (timerIter == steadyTimerMap_.end())
            continue;

        timerIter->second.timer->cancel();
        steadyTimerMap_.erase(tid);
    }

    // Clean System Timers
    const auto systemIter = serviceToSystemTimer_.find(sid);
    if (systemIter == serviceToSystemTimer_.end())
        return;

    for (const auto tid : systemIter->second) {
        const auto timerIter = systemTimerMap_.find(tid);
        if (timerIter == systemTimerMap_.end())
            continue;

        timerIter->second.timer->cancel();
        systemTimerMap_.erase(tid);
    }
}

void UTimerModule::CancelPlayerTimer(const int64_t pid) {
    if (state_ != EModuleState::RUNNING)
        return;

    std::unique_lock lock(timerMutex_);

    // Clean Steady Timers
    const auto steadyIter = playerToSteadyTimer_.find(pid);
    if (steadyIter == playerToSteadyTimer_.end())
        return;

    for (const auto tid : steadyIter->second) {
        const auto timerIter = steadyTimerMap_.find(tid);
        if (timerIter == steadyTimerMap_.end())
            continue;

        timerIter->second.timer->cancel();
        steadyTimerMap_.erase(tid);
    }

    // Clean System Timers
    const auto systemIter = playerToSystemTimer_.find(pid);
    if (systemIter == playerToSystemTimer_.end())
        return;

    for (const auto tid : systemIter->second) {
        const auto timerIter = systemTimerMap_.find(tid);
        if (timerIter == systemTimerMap_.end())
            continue;

        timerIter->second.timer->cancel();
        systemTimerMap_.erase(tid);
    }
}

void UTimerModule::Stop() {
    if (state_ == EModuleState::STOPPED)
        return;

    state_ = EModuleState::STOPPED;

    for (const auto &val : steadyTimerMap_ | std::views::values) {
        val.timer->cancel();
    }
}

void UTimerModule::RemoveSteadyTimer(const int64_t id) {
    std::unique_lock lock(timerMutex_);
    const auto iter = steadyTimerMap_.find(id);
    if (iter == steadyTimerMap_.end())
        return;

    if (iter->second.sid > 0) {
        if (const auto it = serviceToSteadyTimer_.find(iter->second.sid); it != serviceToSteadyTimer_.end()) {
            it->second.erase(id);
        }
    } else {
        if (const auto it = playerToSteadyTimer_.find(iter->second.pid); it != playerToSteadyTimer_.end()) {
            it->second.erase(id);
        }
    }

    steadyTimerMap_.erase(iter);
}

void UTimerModule::RemoveSystemTimer(int64_t id) {
    std::unique_lock lock(timerMutex_);
    const auto iter = systemTimerMap_.find(id);
    if (iter == systemTimerMap_.end())
        return;

    if (iter->second.sid > 0) {
        if (const auto it = serviceToSystemTimer_.find(iter->second.sid); it != serviceToSystemTimer_.end()) {
            it->second.erase(id);
        }
    } else {
        if (const auto it = playerToSystemTimer_.find(iter->second.pid); it != playerToSystemTimer_.end()) {
            it->second.erase(id);
        }
    }

    systemTimerMap_.erase(iter);
}
