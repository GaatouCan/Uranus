#include "../../include/scene/scene_manager.h"
#include "../../include/scene/main_scene.h"
#include "../../include/game_world.h"

#include <ranges>
#include <spdlog/spdlog.h>


SceneManager::SceneManager(GameWorld* world)
    : mWorld(world),
      mNextMainIndex(0)
{
}

SceneManager::~SceneManager()
{
    for (const auto it : mSceneMap | std::views::values)
        delete it;

    mWorkList.clear();
    for (const auto it : mMainSceneList)
        delete it;

    for (auto& th : mThreadList)
    {
        if (th.joinable())
            th.join();
    }
}

void SceneManager::Init()
{
    const auto &cfg = mWorld->GetServerConfig();
    const auto num = cfg["server"]["io_thread"].as<int32_t>();

    for (int32_t idx = 0; idx < num; ++idx)
        mMainSceneList.emplace_back(new MainScene(this, idx));

    for (const auto val : mMainSceneList)
    {
        if (const auto scene = dynamic_cast<MainScene *>(val); scene != nullptr)
        {
            mWorkList.emplace_back(scene->GetIOContext());
            mThreadList.emplace_back([this, scene]
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
    return mWorld;
}

IBaseScene* SceneManager::GetNextMainScene()
{
    if (mMainSceneList.empty())
        throw std::runtime_error("No context node available");

    const auto res = mMainSceneList[mNextMainIndex++];
    mNextMainIndex = mNextMainIndex % mMainSceneList.size();

    return res;
}

IBaseScene* SceneManager::GetScene(const int32_t sid) const
{
    if (sid < 0)
        return nullptr;

    if (sid < NORMAL_SCENE_ID_BEGIN)
    {
        if (sid >= mMainSceneList.size())
            return nullptr;

        return mMainSceneList[sid];
    }

    if (const auto it = mSceneMap.find(sid); it != mSceneMap.end())
        return it->second;

    return nullptr;
}
