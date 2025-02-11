#include "PlayerManager.h"
#include "Player.h"
#include "../common/ProtoType.h"

#include "GameWorld.h"
#include "impl/Package.h"
#include "ConfigManager.h"
#include "system/manager/ManagerSystem.h"


UPlayerManager::UPlayerManager(ManagerSystem *owner)
    : IBaseManager(owner) {
}

UPlayerManager::~UPlayerManager() {
    mPlayerMap.clear();
}

void UPlayerManager::Init() {
}

void UPlayerManager::OnDayChange() {
    for (const auto &plr: mPlayerMap | std::views::values) {
        if (plr != nullptr && plr->IsOnline()) {
            plr->OnDayChange();
        }
    }
}

awaitable<std::shared_ptr<UPlayer> > UPlayerManager::OnPlayerLogin(const std::shared_ptr<Connection> &conn, const PlayerID &id) {
    if (conn == nullptr || std::any_cast<PlayerID>(conn->GetContext()) != id) {
        spdlog::error("{} - Null Connection Pointer Or Player ID Not Equal.", __FUNCTION__);
        co_return nullptr;
    }

    if (const auto plr = FindPlayer(id.local); plr != nullptr) {
        {
            std::scoped_lock lock(mPlayerMutex);
            mPlayerMap.erase(id.local);
        }

        spdlog::info("{} - Player[{}] Over Login", __FUNCTION__, plr->GetFullID());

        plr->GetConnection()->ResetContext();
        plr->GetConnection()->Disconnect();

        plr->TryLeaveScene();

        if (plr->IsOnline()) {
            plr->OnLogout(true, conn->RemoteAddress().to_string());
        }
    }

    const auto plr = std::make_shared<UPlayer>(conn);

    {
        std::scoped_lock lock(mPlayerMutex);
        mPlayerMap[id.local] = plr;
    }

    spdlog::info("{} - New Player[{}] Login", __FUNCTION__, plr->GetFullID());
    co_await plr->OnLogin();

    // 同步玩家缓存数据
    SyncCache(plr);

    co_return plr;
}

void UPlayerManager::OnPlayerLogout(const PlayerID pid) {
    spdlog::info("{} - Player[{}] Logout", __FUNCTION__, pid.ToUInt64());
    if (const auto plr = RemovePlayer(pid.local); plr != nullptr) {
        plr->TryLeaveScene();
        plr->OnLogout();
    }
}

std::shared_ptr<UPlayer> UPlayerManager::FindPlayer(const uint32_t pid) {
    std::shared_lock lock(mPlayerSharedMutex);
    if (const auto it = mPlayerMap.find(pid); it != mPlayerMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<UPlayer> UPlayerManager::RemovePlayer(const uint32_t pid) {
    std::scoped_lock lock(mPlayerMutex);
    if (const auto it = mPlayerMap.find(pid); it != mPlayerMap.end()) {
        auto res = it->second;
        mPlayerMap.erase(it);
        return res;
    }
    return nullptr;
}

void UPlayerManager::SendToList(const std::set<PlayerID> &players, const int32_t id, const std::string_view data) {
    if (id <= MINIMUM_PACKAGE_ID || id >= static_cast<int32_t>(protocol::ProtoType::PROTO_TYPE_MAX))
        return;

    for (const auto [localID, crossID]: players) {
        if (const auto plr = FindPlayer(localID); plr != nullptr && plr->IsOnline()) {
            const auto pkg = dynamic_cast<Package *>(plr->GetConnection()->BuildPackage());
            pkg->SetPackageID(id).SetData(data);
            plr->SendPackage(pkg);
        }
    }
}

void UPlayerManager::SyncCache(const std::shared_ptr<UPlayer> &plr) {
    if (plr == nullptr)
        return;

    FCacheNode node;
    plr->SyncCache(&node);

    SyncCache(node);
}

void UPlayerManager::SyncCache(const uint32_t pid) {
    if (pid < PLAYER_LOCAL_ID_BEGIN || pid > PLAYER_LOCAL_ID_END)
        return;

    const auto plr = FindPlayer(pid);
    SyncCache(plr);
}

void UPlayerManager::SyncCache(const FCacheNode &node) {
    if (!node.pid.IsAvailable())
        return;

    std::scoped_lock lock(mCacheMutex);
    mCacheMap[node.pid.local] = node;
    spdlog::info("{} - Player[{}] Success.", __FUNCTION__, node.pid.ToUInt64());
}

awaitable<std::optional<FCacheNode> > UPlayerManager::FindCacheNode(const PlayerID &pid) {
    if (!pid.IsAvailable())
        co_return std::nullopt;

    if (pid.cross != GetWorld()->GetServerID()) {
        // TODO
        co_return std::nullopt;
    }

    if (const auto plr = FindPlayer(pid.local); plr != nullptr) {
        SyncCache(plr);
    }

    std::shared_lock lock(mCacheSharedMutex);
    if (const auto it = mCacheMap.find(pid.local); it != mCacheMap.end()) {
        co_return std::make_optional(it->second);
    }

    co_return std::nullopt;
}
