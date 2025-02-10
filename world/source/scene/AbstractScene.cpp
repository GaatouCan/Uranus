#include "../../include/scene/AbstractScene.h"
#include "../../include/scene/SceneManager.h"
#include "../../include/scene/BasePlayer.h"
#include "../../include/GameWorld.h"

IAbstractScene::IAbstractScene(USceneManager *owner, const int32_t id)
    : owner_(owner),
      id_(id) {
}

IAbstractScene::~IAbstractScene() {
}

int32_t IAbstractScene::GetSceneID() const {
    return id_;
}

USceneManager *IAbstractScene::GetOwner() const {
    return owner_;
}

UGameWorld *IAbstractScene::GetWorld() const {
    return owner_->GetWorld();
}

void IAbstractScene::PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player) {
    if (player == nullptr)
        return;

    {
        std::shared_lock lock(mutex_);
        if (playerMap_.contains(player->GetLocalID()))
            return;
    }

    // Change Scene From Other Scene
    if (const auto sid = player->GetCurrentSceneID(); sid >= 0 && sid != id_) {
        if (const auto scene = owner_->GetScene(sid); scene != nullptr && scene != this) {
            scene->PlayerLeaveScene(player, true);
        }
    }

    {
        std::unique_lock lock(mutex_);
        playerMap_[player->GetLocalID()] = player;
        spdlog::info("{} - Player[{}] Enter Scene[{}].", __FUNCTION__, player->GetFullID(), id_);
    }

    player->OnEnterScene(this);
}

void IAbstractScene::PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, const bool bChange) {
    if (player == nullptr)
        return;

    {
        std::unique_lock lock(mutex_);

        if (!playerMap_.contains(player->GetLocalID()))
            return;

        playerMap_.erase(player->GetLocalID());
        spdlog::info("{} - Player[{}] Leave Scene[{}].", __FUNCTION__, player->GetFullID(), id_);
    }

    if (!bChange)
        player->OnLeaveScene(this);
}

std::shared_ptr<IBasePlayer> IAbstractScene::GetPlayer(const int32_t pid) const {
    if (pid < kPlayerLocalIDBegin || pid > kPlayerLocalIDEnd)
        return nullptr;

    std::shared_lock lock(mutex_);
    if (const auto it = playerMap_.find(pid); it != playerMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void IAbstractScene::RunInThread(const std::function<awaitable<void>()> &func) const {
    auto &ctx = owner_->GetWorld()->GetIOContext();
    co_spawn(ctx, func, detached);
}

void IAbstractScene::RunInThread(std::function<awaitable<void>()> &&func) const {
    auto &ctx = owner_->GetWorld()->GetIOContext();
    co_spawn(ctx, std::move(func), detached);
}
