#include "../../../include/system/manager/manager_system.h"
#include "../../../include/game_world.h"
#include "../../../include/reactor/global_queue.h"

#include <ranges>
#include <spdlog/spdlog.h>

UManagerSystem::UManagerSystem(UGameWorld *world)
    : ISubSystem(world),
      tick_timer_(GetWorld()->GetIOContext()),
      running_(false) {
}

UManagerSystem::~UManagerSystem() {
    running_ = false;

    for (const auto mgr: manager_map_ | std::views::values)
        delete mgr;
}

void UManagerSystem::Init() {
    for (const auto mgr: manager_map_ | std::views::values) {
        GetWorld()->GetGlobalQueue()->registerReactor(mgr);
    }

    for (const auto mgr: manager_map_ | std::views::values) {
        mgr->Init();
        spdlog::info("\t{} Initialized.", mgr->GetManagerName());
    }

    running_ = true;

    co_spawn(GetWorld()->GetIOContext(), [this]() mutable -> awaitable<void> {
        try {
            tick_point_ = NowTimePoint();
            auto point = tick_point_ + std::chrono::seconds(1);

            while (running_) {
                tick_timer_.expires_at(point);
                co_await tick_timer_.async_wait();

                if (running_)
                    OnTick(point);

                tick_point_ = point;
                point += std::chrono::seconds(1);
            }
        } catch (const std::exception &e) {
            spdlog::error("ManagerSystem::Init() - Failed to run OnTick {}", e.what());
        }
    }, asio::detached);
}


void UManagerSystem::OnTick(const ATimePoint now) {
    for (const auto mgr: manager_map_ | std::views::values) {
        if (!mgr->tick_per_sec_)
            continue;
        mgr->OnTick(now, now - tick_point_);
    }
    tick_point_ = now;

    static int day = 0;

    if (const auto today = std::chrono::floor<std::chrono::days>(now).time_since_epoch().count();
        today > day && day != 0) {
        day = today;
        for (const auto mgr: std::views::values(manager_map_)) {
            mgr->OnDayChange();
        }
    }
}
