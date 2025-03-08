#include "../../include/scene/base_scene.h"
#include "../../include/scene/scene_manager.h"
#include "../../include/scene/base_player.h"
#include "../../include/game_world.h"

#include <ranges>


IBaseScene::IBaseScene(USceneManager *owner, const int32_t id)
    : owner_(owner),
      sceneID_(id) {
}

IBaseScene::~IBaseScene() {
}

int32_t IBaseScene::getSceneID() const {
    return sceneID_;
}

USceneManager *IBaseScene::getOwner() const {
    return owner_;
}

UGameWorld *IBaseScene::getWorld() const {
    return owner_->getWorld();
}

void IBaseScene::playerEnterScene(const std::shared_ptr<IBasePlayer> &player) {
    if (player == nullptr)
        return;

    {
        std::shared_lock lock(mutex_);
        if (playerMap_.contains(player->getLocalID()))
            return;
    }

    // Change Scene From Other Scene
    if (const auto sid = player->getCurrentSceneID(); sid >= 0 && sid != sceneID_) {
        if (const auto scene = owner_->getScene(sid); scene != nullptr && scene != this) {
            scene->playerLeaveScene(player, true);
        }
    }

    {
        std::unique_lock lock(mutex_);
        playerMap_[player->getLocalID()] = player;
        spdlog::info("{} - Player[{}] Enter Scene[{}].", __FUNCTION__, player->getFullID(), sceneID_);
    }

    player->onEnterScene(this);
}

void IBaseScene::playerLeaveScene(const std::shared_ptr<IBasePlayer> &player, const bool is_change) {
    if (player == nullptr)
        return;

    {
        std::unique_lock lock(mutex_);

        if (!playerMap_.contains(player->getLocalID()))
            return;

        playerMap_.erase(player->getLocalID());
        spdlog::info("{} - Player[{}] Leave Scene[{}].", __FUNCTION__, player->getFullID(), sceneID_);
    }

    if (!is_change)
        player->onLeaveScene(this);
}

std::shared_ptr<IBasePlayer> IBaseScene::findPlayer(const int32_t pid) const {
    if (pid < PLAYER_LOCAL_ID_BEGIN || pid > PLAYER_LOCAL_ID_END)
        return nullptr;

    std::shared_lock lock(mutex_);
    if (const auto it = playerMap_.find(pid); it != playerMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void IBaseScene::runInThread(const std::function<awaitable<void>()> &func) const {
    auto &ctx = owner_->getWorld()->GetIOContext();
    co_spawn(ctx, func, detached);
}

void IBaseScene::runInThread(std::function<awaitable<void>()> &&func) const {
    auto &ctx = owner_->getWorld()->GetIOContext();
    co_spawn(ctx, std::move(func), detached);
}

std::set<std::shared_ptr<IBasePlayer>> IBaseScene::getPlayerInScene() const {
    std::set<std::shared_ptr<IBasePlayer>> result;
    std::shared_lock lock(mutex_);
    for (const auto &player : playerMap_ | std::views::values) {
        result.insert(player);
    }
    return result;
}

bool IBaseScene::canDestroy() const {
    return true;
}
