#include "../../include/scene/SceneManager.h"
#include "../../include/scene/MainScene.h"
#include "../../include/GameWorld.h"

#include <ranges>
#include <spdlog/spdlog.h>


USceneManager::USceneManager(UGameWorld* world)
    : world_(world),
      nextIndex_(0)
{
}

USceneManager::~USceneManager()
{
    for (const auto it : sceneMap_ | std::views::values)
        delete it;

    workVec_.clear();
    for (const auto it : mainSceneVec_)
        delete it;

    for (auto& th : threadVec_)
    {
        if (th.joinable())
            th.join();
    }
}

void USceneManager::Init()
{
    const auto &cfg = world_->GetServerConfig();
    const auto num = cfg["server"]["io_thread"].as<int32_t>();

    for (int32_t idx = 0; idx < num; ++idx)
        mainSceneVec_.emplace_back(new UMainScene(this, idx));

    for (const auto val : mainSceneVec_)
    {
        if (const auto scene = dynamic_cast<UMainScene*>(val); scene != nullptr)
        {
            workVec_.emplace_back(scene->GetIOContext());
            threadVec_.emplace_back([this, scene]
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
    return world_;
}

IAbstractScene* USceneManager::GetNextMainScene()
{
    if (mainSceneVec_.empty())
        throw std::runtime_error("No context node available");

    const auto res = mainSceneVec_[nextIndex_++];
    nextIndex_ = nextIndex_ % mainSceneVec_.size();

    return res;
}

IAbstractScene* USceneManager::GetScene(const int32_t sid) const
{
    if (sid < 0)
        return nullptr;

    if (sid < kNormalSceneIDBegin)
    {
        if (sid >= mainSceneVec_.size())
            return nullptr;

        return mainSceneVec_[sid];
    }

    if (const auto it = sceneMap_.find(sid); it != sceneMap_.end())
        return it->second;

    return nullptr;
}
