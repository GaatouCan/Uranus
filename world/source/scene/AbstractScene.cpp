#include "../../include/scene/AbstractScene.h"
#include "../../include/scene/SceneManager.h"
#include "../../include/scene/BasePlayer.h"
#include "../../include/GameWorld.h"

IAbstractScene::IAbstractScene(USceneManager *owner, const uint32_t id)
    : mOwner(owner),
      mSceneID(id) {
}

IAbstractScene::~IAbstractScene() {
}

uint32_t IAbstractScene::GetSceneID() const {
    return mSceneID;
}

USceneManager *IAbstractScene::GetOwner() const {
    return mOwner;
}

UGameWorld *IAbstractScene::GetWorld() const {
    return mOwner->GetWorld();
}

void IAbstractScene::PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player) {
    if (player == nullptr)
        return;

    {
        std::shared_lock lock(mSharedMutex);
        if (mPlayerMap.contains(player->GetLocalID()))
            return;
    }

    // Change Scene From Other Scene
    if (const auto sid = player->GetCurrentSceneID(); sid >= 0 && sid != mSceneID) {
        if (const auto scene = mOwner->GetScene(sid); scene != nullptr && scene != this) {
            scene->PlayerLeaveScene(player, true);
        }
    }

    {
        std::scoped_lock lock(mMutex);
        mPlayerMap[player->GetLocalID()] = player;
        spdlog::info("{} - Player[{}] Enter Scene[{}].", __FUNCTION__, player->GetFullID(), mSceneID);
    }

    player->OnEnterScene(this);
}

void IAbstractScene::PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, const bool bChange) {
    if (player == nullptr)
        return;

    {
        std::scoped_lock lock(mMutex);

        if (!mPlayerMap.contains(player->GetLocalID()))
            return;

        mPlayerMap.erase(player->GetLocalID());
        spdlog::info("{} - Player[{}] Leave Scene[{}].", __FUNCTION__, player->GetFullID(), mSceneID);
    }

    if (!bChange)
        player->OnLeaveScene(this);
}

std::shared_ptr<IBasePlayer> IAbstractScene::GetPlayer(const uint32_t pid) const {
    if (pid < kPlayerLocalIDBegin || pid > kPlayerLocalIDEnd)
        return nullptr;

    std::shared_lock lock(mSharedMutex);
    if (const auto it = mPlayerMap.find(pid); it != mPlayerMap.end()) {
        return it->second;
    }
    return nullptr;
}

void IAbstractScene::RunInThread(const std::function<awaitable<void>()> &func) const {
    auto &ctx = mOwner->GetWorld()->GetIOContext();
    co_spawn(ctx, func, detached);
}

void IAbstractScene::RunInThread(std::function<awaitable<void>()> &&func) const {
    auto &ctx = mOwner->GetWorld()->GetIOContext();
    co_spawn(ctx, std::move(func), detached);
}
