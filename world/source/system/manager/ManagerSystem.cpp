#include "../../../include/system/manager/ManagerSystem.h"
#include "../../../include/system/timer/TimerSystem.h"
#include "../../../include/GameWorld.h"
#include "../../../include/reactor/GlobalQueue.h"

#include <ranges>
#include <spdlog/spdlog.h>

UManagerSystem::UManagerSystem(UGameWorld *world)
    : ISubSystem(world), mTimerID() {
}

UManagerSystem::~UManagerSystem() {
    for (const auto mgr: mManagerMap | std::views::values)
        delete mgr;
}

void UManagerSystem::Init() {
    for (const auto mgr: mManagerMap | std::views::values) {
        GetWorld()->GetGlobalQueue()->RegisterReactor(mgr);
    }

    for (const auto mgr: mManagerMap | std::views::values) {
        mgr->Init();
        spdlog::info("\t{} Initialized.", mgr->GetManagerName());
    }

    if (const auto sys = GetWorld()->GetSystem<UTimerSystem>(); sys != nullptr) {
        if (const auto value = sys->SetTimerTo(std::chrono::seconds(1), std::chrono::seconds(1), true, &UManagerSystem::OnTick, this); value.has_value())
            mTimerID = value.value();
        else {
            spdlog::error("{} - Failed to set timer for UManagerSystem", __FUNCTION__);
            GetWorld()->Shutdown();
            exit(-1);
        }
    }
}


void UManagerSystem::OnTick(ATimePoint now) {
    // spdlog::debug("{} - {}", __FUNCTION__, ToUnixTime(now));
    for (const auto mgr: std::views::values(mManagerMap)) {
        if (!mgr->bTick) continue;
        mgr->PushTask<IBaseManager>(&IBaseManager::OnTick, now);
    }

    static int day = 0;

    if (const auto today = std::chrono::floor<std::chrono::days>(now).time_since_epoch().count(); today > day && day != 0) {
        day = today;
        for (const auto mgr: std::views::values(mManagerMap)) {
            mgr->PushTask<IBaseManager>(&IBaseManager::OnDayChange);
        }
    }
}
