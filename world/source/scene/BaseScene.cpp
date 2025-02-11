#include "../../include/scene/BaseScene.h"
#include "../../include/scene/SceneManager.h"
#include "../../include/scene/BasePlayer.h"
#include "../../include/GameWorld.h"

IBaseScene::IBaseScene(SceneManager *owner, const int32_t id)
    : owner_(owner),
      id_(id) {
}

IBaseScene::~IBaseScene() {
}

int32_t IBaseScene::GetSceneID() const {
    return id_;
}

SceneManager *IBaseScene::GetOwner() const {
    return owner_;
}

GameWorld *IBaseScene::GetWorld() const {
    return owner_->GetWorld();
}

void IBaseScene::PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player) {
    if (player == nullptr)
        return;

    {
        std::shared_lock lock(mutex_);
        if (player_map_.contains(player->GetLocalID()))
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
        player_map_[player->GetLocalID()] = player;
        spdlog::info("{} - Player[{}] Enter Scene[{}].", __FUNCTION__, player->GetFullID(), id_);
    }

    player->OnEnterScene(this);
}

void IBaseScene::PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, const bool is_change) {
    if (player == nullptr)
        return;

    {
        std::unique_lock lock(mutex_);

        if (!player_map_.contains(player->GetLocalID()))
            return;

        player_map_.erase(player->GetLocalID());
        spdlog::info("{} - Player[{}] Leave Scene[{}].", __FUNCTION__, player->GetFullID(), id_);
    }

    if (!is_change)
        player->OnLeaveScene(this);
}

std::shared_ptr<IBasePlayer> IBaseScene::GetPlayer(const int32_t pid) const {
    if (pid < PLAYER_LOCAL_ID_BEGIN || pid > PLAYER_LOCAL_ID_END)
        return nullptr;

    std::shared_lock lock(mutex_);
    if (const auto it = player_map_.find(pid); it != player_map_.end()) {
        return it->second;
    }
    return nullptr;
}

void IBaseScene::RunInThread(const std::function<awaitable<void>()> &func) const {
    auto &ctx = owner_->GetWorld()->GetIOContext();
    co_spawn(ctx, func, detached);
}

void IBaseScene::RunInThread(std::function<awaitable<void>()> &&func) const {
    auto &ctx = owner_->GetWorld()->GetIOContext();
    co_spawn(ctx, std::move(func), detached);
}
