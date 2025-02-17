#include "../../../include/system/manager/manager_system.h"
#include "../../../include/game_world.h"
#include "../../../include/reactor/global_queue.h"

#include <ranges>
#include <spdlog/spdlog.h>

ManagerSystem::ManagerSystem(GameWorld *world)
    : ISubSystem(world),
      mTickTimer(GetWorld()->GetIOContext()),
      bRunning(false) {
}

ManagerSystem::~ManagerSystem() {
    bRunning = false;

    for (const auto mgr: mManagerMap | std::views::values)
        delete mgr;
}

void ManagerSystem::Init() {
    for (const auto mgr: mManagerMap | std::views::values) {
        GetWorld()->GetGlobalQueue()->RegisterReactor(mgr);
    }

    for (const auto mgr: mManagerMap | std::views::values) {
        mgr->Init();
        spdlog::info("\t{} Initialized.", mgr->GetManagerName());
    }

    bRunning = true;

    co_spawn(GetWorld()->GetIOContext(), [this]() mutable -> awaitable<void> {
        try {
            auto point = NowTimePoint() + std::chrono::seconds(1);
            while (bRunning) {
                mTickTimer.expires_at(point);
                point += std::chrono::seconds(1);

                co_await mTickTimer.async_wait();

                if (bRunning)
                    OnTick(point);
            }
        } catch (const std::exception &e) {
            spdlog::error("ManagerSystem::Init() - Failed to run OnTick {}", e.what());
        }
    }, asio::detached);
}


void ManagerSystem::OnTick(TimePoint now) {
    spdlog::debug("{} - {}", __FUNCTION__, utils::ToUnixTime(now));
    for (const auto mgr: std::views::values(mManagerMap)) {
        if (!mgr->bTick) continue;
        mgr->PushTask<IBaseManager>(&IBaseManager::OnTick, now);
    }

    static int day = 0;

    if (const auto today = std::chrono::floor<std::chrono::days>(now).time_since_epoch().count();
        today > day && day != 0) {
        day = today;
        for (const auto mgr: std::views::values(mManagerMap)) {
            mgr->PushTask<IBaseManager>(&IBaseManager::OnDayChange);
        }
    }
}
