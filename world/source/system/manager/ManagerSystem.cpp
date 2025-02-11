#include "../../../include/system/manager/ManagerSystem.h"
#include "../../../include/system/timer/TimerSystem.h"
#include "../../../include/GameWorld.h"
#include "../../../include/reactor/GlobalQueue.h"

#include <ranges>
#include <spdlog/spdlog.h>

ManagerSystem::ManagerSystem(GameWorld *world)
    : ISubSystem(world), timer_id_() {
}

ManagerSystem::~ManagerSystem() {
    for (const auto mgr: manager_map_ | std::views::values)
        delete mgr;
}

void ManagerSystem::Init() {
    for (const auto mgr: manager_map_ | std::views::values) {
        GetWorld()->GetGlobalQueue()->RegisterReactor(mgr);
    }

    for (const auto mgr: manager_map_ | std::views::values) {
        mgr->Init();
        spdlog::info("\t{} Initialized.", mgr->GetManagerName());
    }

    if (const auto sys = GetWorld()->GetSystem<TimerSystem>(); sys != nullptr) {
        if (const auto value = sys->SetTimerTo(std::chrono::seconds(1), std::chrono::seconds(1), true, &ManagerSystem::OnTick, this); value.has_value())
            timer_id_ = value.value();
        else {
            spdlog::error("{} - Failed to set timer for UManagerSystem", __FUNCTION__);
            GetWorld()->Shutdown();
            exit(-1);
        }
    }
}


void ManagerSystem::OnTick(TimePoint now) {
    // spdlog::debug("{} - {}", __FUNCTION__, ToUnixTime(now));
    for (const auto mgr: std::views::values(manager_map_)) {
        if (!mgr->tick_) continue;
        mgr->PushTask<IBaseManager>(&IBaseManager::OnTick, now);
    }

    static int day = 0;

    if (const auto today = std::chrono::floor<std::chrono::days>(now).time_since_epoch().count(); today > day && day != 0) {
        day = today;
        for (const auto mgr: std::views::values(manager_map_)) {
            mgr->PushTask<IBaseManager>(&IBaseManager::OnDayChange);
        }
    }
}
