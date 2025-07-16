#include "TimerModule.h"
#include "Server.h"
#include "Gateway/Gateway.h"
#include "Gateway/PlayerAgent.h"
#include "Service/ServiceModule.h"

#include <spdlog/spdlog.h>


UTimerModule::UTimerModule(UServer *server)
    : Super(server),
      mNextID(1) {
}

UTimerModule::~UTimerModule() {
    Stop();
}

int64_t UTimerModule::SetTimer(const int32_t sid, const int64_t pid, const ATimerTask &task, int delay, int rate) {
    const auto id = AllocateTimerID();
    if (id < 0)
        return -1;

    std::unique_lock lock(mTimerMutex);

    FTimerNode node {
        sid,
        pid,
        make_shared<ASteadyTimer>(GetServer()->GetIOContext())
    };

    mTimerMap.emplace(id, std::move(node));

    if (sid > 0) {
        if (!mServiceToTimer.contains(sid)) {
            mServiceToTimer[sid] = absl::flat_hash_set<int64_t>();
        }
        mServiceToTimer[sid].emplace(id);
    } else {
        if (!mPlayerToTimer.contains(pid)) {
            mPlayerToTimer[pid] = absl::flat_hash_set<int64_t>();
        }
        mPlayerToTimer[pid].emplace(id);
    }

    co_spawn(GetServer()->GetIOContext(), [this, timer = mTimerMap[id].timer, id, sid, pid, delay, rate, task]() -> awaitable<void> {
        try {
            auto point = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay * 100);

            do {
                timer->expires_at(point);
                if (rate > 0) {
                    point += std::chrono::milliseconds(rate * 100);
                }

                if (auto [ec] = co_await timer->async_wait(); ec) {
                    SPDLOG_DEBUG("{:<20} - Timer[{}] Canceled", "UTimerModule::SetTimer()", id);
                    co_return;
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

            } while (rate > 0 && mState == EModuleState::RUNNING);
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{:<20} - Exception: {}", "UTimerModule::SetTimer", e.what());
        }

        RecycleTimerID(id);
        RemoveTimer(id);
    }, detached);

    return id;
}

void UTimerModule::CancelTimer(const int64_t id) {
    if (mState != EModuleState::RUNNING)
        return;

    std::unique_lock lock(mTimerMutex);
    const auto iter = mTimerMap.find(id);
    if (iter == mTimerMap.end())
        return;

    iter->second.timer->cancel();

    if (iter->second.sid > 0) {
        if (const auto it = mServiceToTimer.find(iter->second.sid); it != mServiceToTimer.end()) {
            it->second.erase(id);
        }
    } else {
        if (const auto it = mPlayerToTimer.find(iter->second.pid); it != mPlayerToTimer.end()) {
            it->second.erase(id);
        }
    }

    mTimerMap.erase(iter);
}

void UTimerModule::CancelServiceTimer(const int32_t sid) {
    if (mState != EModuleState::RUNNING)
        return;

    std::unique_lock lock(mTimerMutex);
    const auto it = mServiceToTimer.find(sid);
    if (it == mServiceToTimer.end())
        return;

    for (const auto tid : it->second) {
        const auto timerIter = mTimerMap.find(tid);
        if (timerIter == mTimerMap.end())
            continue;

        timerIter->second.timer->cancel();
        mTimerMap.erase(tid);
    }
}

void UTimerModule::CancelPlayerTimer(const int64_t pid) {
    if (mState != EModuleState::RUNNING)
        return;

    std::unique_lock lock(mTimerMutex);
    const auto it = mPlayerToTimer.find(pid);
    if (it == mPlayerToTimer.end())
        return;

    for (const auto tid : it->second) {
        const auto timerIter = mTimerMap.find(tid);
        if (timerIter == mTimerMap.end())
            continue;

        timerIter->second.timer->cancel();
        mTimerMap.erase(tid);
    }
}

void UTimerModule::Stop() {
    if (mState == EModuleState::STOPPED)
        return;

    mState = EModuleState::STOPPED;

    for (const auto &val : mTimerMap | std::views::values) {
        val.timer->cancel();
    }
}

int64_t UTimerModule::AllocateTimerID() {
    if (mState != EModuleState::RUNNING)
        return -1;

    std::unique_lock lock(mIDMutex);

    if (!mRecycledID.empty()) {
        const int64_t id = mRecycledID.front();
        mRecycledID.pop();
        return id;
    }

    if (mNextID < 0) {
        // SPDLOG_ERROR("{:<20} - Service ID Allocator Overflow", __FUNCTION__);
        return -1;
    }

    return mNextID++;
}

void UTimerModule::RecycleTimerID(const int64_t id) {
    std::unique_lock lock(mIDMutex);
    mRecycledID.push(id);
}

void UTimerModule::RemoveTimer(const int64_t id) {
    std::unique_lock lock(mTimerMutex);
    const auto iter = mTimerMap.find(id);
    if (iter == mTimerMap.end())
        return;

    if (iter->second.sid > 0) {
        if (const auto it = mServiceToTimer.find(iter->second.sid); it != mServiceToTimer.end()) {
            it->second.erase(id);
        }
    } else {
        if (const auto it = mPlayerToTimer.find(iter->second.pid); it != mPlayerToTimer.end()) {
            it->second.erase(id);
        }
    }

    mTimerMap.erase(iter);
}
