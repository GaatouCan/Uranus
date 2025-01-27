#include "../../include/scene/SceneManager.h"
#include "../../include/scene/MainScene.h"
#include "../../include/GameWorld.h"

#include <ranges>
#include <spdlog/spdlog.h>


USceneManager::USceneManager(UGameWorld* world)
    : mWorld(world),
      mNextIndex(0)
{
}

USceneManager::~USceneManager()
{
    for (const auto it : mSceneMap | std::views::values)
        delete it;

    mWorkVec.clear();
    for (const auto it : mMainVec)
        delete it;

    for (auto& th : mThreadVec)
    {
        if (th.joinable())
            th.join();
    }
}

void USceneManager::Init()
{
    const auto &cfg = mWorld->GetServerConfig();
    const auto num = cfg["server"]["io_thread"].as<int32_t>();

    for (int32_t idx = 0; idx < num; ++idx)
        mMainVec.emplace_back(new UMainScene(this, idx));

    for (const auto val : mMainVec)
    {
        if (const auto scene = dynamic_cast<UMainScene*>(val); scene != nullptr)
        {
            mWorkVec.emplace_back(scene->GetIOContext());
            mThreadVec.emplace_back([this, scene]
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

UGameWorld* USceneManager::GetWorld() const
{
    return mWorld;
}

IAbstractScene* USceneManager::GetNextMainScene()
{
    if (mMainVec.empty())
        throw std::runtime_error("No context node available");

    const auto res = mMainVec[mNextIndex++];
    mNextIndex = mNextIndex % mMainVec.size();

    return res;
}

IAbstractScene* USceneManager::GetScene(const int32_t sid) const
{
    if (sid < kNormalSceneIDBegin)
    {
        if (sid < 0 || sid >= mMainVec.size())
            return nullptr;

        return mMainVec[sid];
    }

    if (const auto it = mSceneMap.find(sid); it != mSceneMap.end())
        return it->second;

    return nullptr;
}
