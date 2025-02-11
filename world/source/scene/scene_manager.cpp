#include "../../include/scene/SceneManager.h"
#include "../../include/scene/MainScene.h"
#include "../../include/GameWorld.h"

#include <ranges>
#include <spdlog/spdlog.h>


SceneManager::SceneManager(GameWorld* world)
    : world_(world),
      next_main_index_(0)
{
}

SceneManager::~SceneManager()
{
    for (const auto it : scene_map_ | std::views::values)
        delete it;

    worker_vec_.clear();
    for (const auto it : main_scene_vec_)
        delete it;

    for (auto& th : thread_vec_)
    {
        if (th.joinable())
            th.join();
    }
}

void SceneManager::Init()
{
    const auto &cfg = world_->GetServerConfig();
    const auto num = cfg["server"]["io_thread"].as<int32_t>();

    for (int32_t idx = 0; idx < num; ++idx)
        main_scene_vec_.emplace_back(new MainScene(this, idx));

    for (const auto val : main_scene_vec_)
    {
        if (const auto scene = dynamic_cast<MainScene*>(val); scene != nullptr)
        {
            worker_vec_.emplace_back(scene->GetIOContext());
            thread_vec_.emplace_back([this, scene]
            {
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

GameWorld* SceneManager::GetWorld() const
{
    return world_;
}

IBaseScene* SceneManager::GetNextMainScene()
{
    if (main_scene_vec_.empty())
        throw std::runtime_error("No context node available");

    const auto res = main_scene_vec_[next_main_index_++];
    next_main_index_ = next_main_index_ % main_scene_vec_.size();

    return res;
}

IBaseScene* SceneManager::GetScene(const int32_t sid) const
{
    if (sid < 0)
        return nullptr;

    if (sid < kNormalSceneIDBegin)
    {
        if (sid >= main_scene_vec_.size())
            return nullptr;

        return main_scene_vec_[sid];
    }

    if (const auto it = scene_map_.find(sid); it != scene_map_.end())
        return it->second;

    return nullptr;
}
