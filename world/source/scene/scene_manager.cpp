#include "../../include/scene/scene_manager.h"
#include "../../include/scene/main_scene.h"
#include "../../include/game_world.h"

#include <ranges>
#include <spdlog/spdlog.h>


SceneManager::SceneManager(GameWorld *world)
    : world_(world),
      next_main_idx_(0) {
}

SceneManager::~SceneManager() {
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

void SceneManager::Init() {
    const auto &cfg = world_->GetServerConfig();
    const auto num = cfg["server"]["io_thread"].as<int32_t>();

    for (int32_t idx = 0; idx < num; ++idx)
        main_scene_list_.emplace_back(new MainScene(this, idx));

    for (const auto val: main_scene_list_) {
        if (const auto scene = dynamic_cast<MainScene *>(val); scene != nullptr) {
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

    spdlog::info("Started With {} Thread(s).", num);
}

GameWorld *SceneManager::GetWorld() const {
    return world_;
}

IBaseScene *SceneManager::GetNextMainScene() {
    if (main_scene_list_.empty())
        throw std::runtime_error("No context node available");

    const auto res = main_scene_list_[next_main_idx_++];
    next_main_idx_ = next_main_idx_ % main_scene_list_.size();

    return res;
}

IBaseScene *SceneManager::GetScene(const int32_t sid) const {
    if (sid < 0)
        return nullptr;

    if (sid < NORMAL_SCENE_ID_BEGIN) {
        if (sid >= main_scene_list_.size())
            return nullptr;

        return main_scene_list_[sid];
    }

    if (const auto it = scene_map_.find(sid); it != scene_map_.end())
        return it->second;

    return nullptr;
}
