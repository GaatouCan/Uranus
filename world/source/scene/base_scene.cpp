#include "../../include/scene/base_scene.h"
#include "../../include/scene/scene_manager.h"
#include "../../include/scene/base_player.h"
#include "../../include/game_world.h"

#include <ranges>


IBaseScene::IBaseScene(SceneManager *owner, const int32_t id)
    : mOwner(owner),
      mSceneID(id) {
}

IBaseScene::~IBaseScene() {
}

int32_t IBaseScene::GetSceneID() const {
    return mSceneID;
}

SceneManager *IBaseScene::GetOwner() const {
    return mOwner;
}

GameWorld *IBaseScene::GetWorld() const {
    return mOwner->GetWorld();
}

void IBaseScene::PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player) {
    if (player == nullptr)
        return;

    {
        std::shared_lock lock(mMutex);
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
        std::unique_lock lock(mMutex);
        mPlayerMap[player->GetLocalID()] = player;
        spdlog::info("{} - Player[{}] Enter Scene[{}].", __FUNCTION__, player->GetFullID(), mSceneID);
    }

    player->OnEnterScene(this);
}

void IBaseScene::PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, const bool is_change) {
    if (player == nullptr)
        return;

    {
        std::unique_lock lock(mMutex);

        if (!mPlayerMap.contains(player->GetLocalID()))
            return;

        mPlayerMap.erase(player->GetLocalID());
        spdlog::info("{} - Player[{}] Leave Scene[{}].", __FUNCTION__, player->GetFullID(), mSceneID);
    }

    if (!is_change)
        player->OnLeaveScene(this);
}

std::shared_ptr<IBasePlayer> IBaseScene::GetPlayer(const int32_t pid) const {
    if (pid < PLAYER_LOCAL_ID_BEGIN || pid > PLAYER_LOCAL_ID_END)
        return nullptr;

    std::shared_lock lock(mMutex);
    if (const auto it = mPlayerMap.find(pid); it != mPlayerMap.end()) {
        return it->second;
    }
    return nullptr;
}

void IBaseScene::RunInThread(const std::function<awaitable<void>()> &func) const {
    auto &ctx = mOwner->GetWorld()->GetIOContext();
    co_spawn(ctx, func, detached);
}

void IBaseScene::RunInThread(std::function<awaitable<void>()> &&func) const {
    auto &ctx = mOwner->GetWorld()->GetIOContext();
    co_spawn(ctx, std::move(func), detached);
}

std::set<std::shared_ptr<IBasePlayer>> IBaseScene::GetPlayerInScene() const {
    std::set<std::shared_ptr<IBasePlayer>> result;
    std::shared_lock lock(mMutex);
    for (const auto &player : mPlayerMap | std::views::values) {
        result.insert(player);
    }
    return result;
}
