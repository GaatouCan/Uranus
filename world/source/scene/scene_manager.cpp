#include "../../include/scene/scene_manager.h"
#include "../../include/scene/main_scene.h"
#include "../../include/game_world.h"

#include <ranges>
#include <spdlog/spdlog.h>


USceneManager::USceneManager(UGameWorld *world)
    : world_(world),
      next_main_idx_(0),
      next_scene_id_(NORMAL_SCENE_ID_BEGIN + 1),
      tick_timer_(world_->GetIOContext()),
      running_(false) {
}

USceneManager::~USceneManager() {
    running_ = false;

    for (const auto it: scene_map_ | std::views::values)
        delete it;

    work_list_.clear();
    for (const auto it: main_scene_list_)
        delete it;

    for (auto &th: thread_list_) {
        if (th.joinable())
            th.join();
    }
}

int32_t USceneManager::GenerateSceneID() {
    int32_t id;

    std::shared_lock lock(scene_mtx_);
    if (scene_map_.size() >= (NORMAL_SCENE_ID_END - NORMAL_SCENE_ID_BEGIN - 1))
        return -1;

    do {
        if (next_scene_id_ >= NORMAL_SCENE_ID_END)
            next_scene_id_ = NORMAL_SCENE_ID_BEGIN + 1;
        id = next_scene_id_++;
    } while (scene_map_.contains(id));

    return id;
}

void USceneManager::EmplaceScene(IBaseScene *scene) {
    if (scene == nullptr)
        return;

    std::unique_lock lock(scene_mtx_);
    scene_map_[scene->GetSceneID()] = scene;
}

void USceneManager::CollectScene(const ATimePoint time) {
    std::unique_lock lock(scene_mtx_);
    constexpr auto zero_time_point = ATimePoint();

    for (auto it = scene_map_.begin(); it != scene_map_.end();) {
        if (it->second->destroy_time_point_ > zero_time_point && it->second->destroy_time_point_ < time && it->second->CanDestroy()) {
            const auto *scene = it->second;
            it = scene_map_.erase(it);
            delete scene;
            continue;
        }
        ++it;
    }
}

void USceneManager::Init() {
    const auto &cfg = world_->GetServerConfig();
    const auto num = cfg["server"]["io_thread"].as<int32_t>();

    for (int32_t idx = 0; idx < num; ++idx)
        main_scene_list_.emplace_back(new UMainScene(this, idx));

    for (const auto val: main_scene_list_) {
        if (const auto scene = dynamic_cast<UMainScene *>(val); scene != nullptr) {
            work_list_.emplace_back(scene->GetIOContext());
            thread_list_.emplace_back([this, scene] {
                asio::signal_set signals(scene->GetIOContext(), SIGINT, SIGTERM);
                signals.async_wait([scene](auto, auto) {
                    scene->GetIOContext().stop();
                    spdlog::info("Main Scene[{}] Shutdown.", scene->GetSceneID());
                });

                scene->SetThreadID(std::this_thread::get_id());
                scene->GetIOContext().run();
            });
        }
    }
    running_ = true;

    spdlog::info("Started With {} Thread(s).", num);

    co_spawn(GetWorld()->GetIOContext(), [this]() mutable -> awaitable<void> {
        try {
            auto point = NowTimePoint() + std::chrono::seconds(1);
            while (running_) {
                tick_timer_.expires_at(point);
                co_await tick_timer_.async_wait();

                if (running_)
                    CollectScene(point);

                point += std::chrono::seconds(1);
            }
        } catch (const std::exception &e) {
            spdlog::error("SceneManager::Init() - Failed to run CollectScene {}", e.what());
        }
    }, asio::detached);
}

UGameWorld *USceneManager::GetWorld() const {
    return world_;
}

IBaseScene *USceneManager::GetNextMainScene() {
    if (main_scene_list_.empty())
        throw std::runtime_error("No context node available");

    const auto res = main_scene_list_[next_main_idx_++];
    next_main_idx_ = next_main_idx_ % main_scene_list_.size();

    return res;
}

IBaseScene *USceneManager::GetScene(const int32_t sid) const {
    if (sid < 0)
        return nullptr;

    if (sid < NORMAL_SCENE_ID_BEGIN) {
        if (sid >= main_scene_list_.size())
            return nullptr;

        return main_scene_list_[sid];
    }

    if (sid <= NORMAL_SCENE_ID_BEGIN || sid >= NORMAL_SCENE_ID_END)
        return nullptr;

    std::shared_lock lock(scene_mtx_);
    if (const auto it = scene_map_.find(sid); it != scene_map_.end())
        return it->second;

    return nullptr;
}
