#include "Scene.h"
#include "BasePlayer.h"
#include "SceneSystem.h"

UScene::UScene(const uint32_t id)
    : mSceneID(id) {
}

UScene::~UScene() {
}

void UScene::SetSceneID(const uint32_t id) {
    mSceneID = id;
}

uint32_t UScene::GetSceneID() const {
    return mSceneID;
}

void UScene::PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player) {
    if (player == nullptr)
        return;

    {
        std::shared_lock lock(mSharedMutex);
        if (mPlayerMap.contains(player->GetLocalID()))
            return;
    }

    // Change Scene From Other Scene
    if (const auto otherSceneID = player->GetCurrentSceneID(); otherSceneID != 0 && otherSceneID != mSceneID) {
        if (const auto sys = UGameWorld::Instance().GetSystem<USceneSystem>(); sys != nullptr) {
            if (const auto scene = sys->GetSceneByID(otherSceneID); scene != nullptr && scene != this) {
                scene->PlayerLeaveScene(player, true);
            }
        }
    }

    {
        std::scoped_lock lock(mMutex);
        mPlayerMap[player->GetLocalID()] = player;
        spdlog::info("{} - Player[{}] Enter Scene[{}].", __FUNCTION__, player->GetFullID(), mSceneID);
    }
    player->OnEnterScene(this);
}

void UScene::PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, const bool bChange) {
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

std::shared_ptr<IBasePlayer> UScene::GetPlayer(const uint32_t pid) const {
    if (pid < kPlayerLocalIDBegin || pid > kPlayerLocalIDEnd)
        return nullptr;

    std::shared_lock lock(mSharedMutex);
    if (const auto it = mPlayerMap.find(pid); it != mPlayerMap.end()) {
        return it->second;
    }
    return nullptr;
}

void UScene::BroadCast(IPackage *pkg, const std::set<uint32_t> &except) {
    if (pkg == nullptr)
        return;

    std::shared_lock lock(mSharedMutex);
    for (const auto &[pid, plr]: mPlayerMap) {
        if (except.contains(pid))
            continue;

        const auto tPkg = plr->BuildPackage();
        tPkg->CopyFrom(pkg);
        plr->SendPackage(tPkg);
    }
}
