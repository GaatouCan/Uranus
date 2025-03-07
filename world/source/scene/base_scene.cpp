#include "../../include/scene/base_scene.h"
#include "../../include/scene/scene_manager.h"
#include "../../include/scene/base_player.h"
#include "../../include/game_world.h"

#include <ranges>


IBaseScene::IBaseScene(USceneManager *owner, const int32_t id)
    : owner_(owner),
      scene_id_(id) {
}

IBaseScene::~IBaseScene() {
}

int32_t IBaseScene::GetSceneID() const {
    return scene_id_;
}

USceneManager *IBaseScene::GetOwner() const {
    return owner_;
}

UGameWorld *IBaseScene::GetWorld() const {
    return owner_->GetWorld();
}

void IBaseScene::PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player) {
    if (player == nullptr)
        return;

    {
        std::shared_lock lock(mtx_);
        if (player_map_.contains(player->GetLocalID()))
            return;
    }

    // Change Scene From Other Scene
    if (const auto sid = player->GetCurrentSceneID(); sid >= 0 && sid != scene_id_) {
        if (const auto scene = owner_->GetScene(sid); scene != nullptr && scene != this) {
            scene->PlayerLeaveScene(player, true);
        }
    }

    {
        std::unique_lock lock(mtx_);
        player_map_[player->GetLocalID()] = player;
        spdlog::info("{} - Player[{}] Enter Scene[{}].", __FUNCTION__, player->GetFullID(), scene_id_);
    }

    player->OnEnterScene(this);
}

void IBaseScene::PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, const bool is_change) {
    if (player == nullptr)
        return;

    {
        std::unique_lock lock(mtx_);

        if (!player_map_.contains(player->GetLocalID()))
            return;

        player_map_.erase(player->GetLocalID());
        spdlog::info("{} - Player[{}] Leave Scene[{}].", __FUNCTION__, player->GetFullID(), scene_id_);
    }

    if (!is_change)
        player->OnLeaveScene(this);
}

std::shared_ptr<IBasePlayer> IBaseScene::GetPlayer(const int32_t pid) const {
    if (pid < PLAYER_LOCAL_ID_BEGIN || pid > PLAYER_LOCAL_ID_END)
        return nullptr;

    std::shared_lock lock(mtx_);
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

std::set<std::shared_ptr<IBasePlayer>> IBaseScene::GetPlayerInScene() const {
    std::set<std::shared_ptr<IBasePlayer>> result;
    std::shared_lock lock(mtx_);
    for (const auto &player : player_map_ | std::views::values) {
        result.insert(player);
    }
    return result;
}

bool IBaseScene::CanDestroy() const {
    return true;
}
