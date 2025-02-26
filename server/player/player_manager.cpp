#include "player_manager.h"

#include "player.h"
#include "../common/proto_type.h"

#include "game_world.h"
#include "impl/package.h"
#include "config_manager.h"
#include "system/manager/manager_system.h"
#include "system/database/database_system.h"
#include "system/database/serializer.h"

#include "player_cache.orm.h"


static_assert(ByteArray::kPODType<CacheNode>, "CacheNode Is Not POD Type");

PlayerManager::PlayerManager(ManagerSystem *owner)
    : IBaseManager(owner) {

}

PlayerManager::~PlayerManager() {
    mPlayerMap.clear();
}

void PlayerManager::Init() {
    if (const auto sys = GetWorld()->GetSystem<DatabaseSystem>(); sys != nullptr) {
        sys->SyncSelect("player_cache", "", [&](mysqlx::Row row) {
            orm::DBTable_PlayerCache table;
            table.Read(row);

            const PlayerID pid(table.pid);
            if (!pid.IsAvailable())
                return;

            if (auto *node = &mCacheMap[pid.local]; node != nullptr)
                table.cache >> node;
        });
    }
    bTick = true;
}

void PlayerManager::OnDayChange() {
    for (const auto &plr: mPlayerMap | std::views::values) {
        if (plr != nullptr && plr->IsOnline()) {
            plr->OnDayChange();
        }
    }
}

awaitable<std::shared_ptr<Player>> PlayerManager::OnPlayerLogin(const std::shared_ptr<Connection> &conn, const PlayerID &id) {
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

    const auto plr = std::make_shared<Player>(conn);

    {
        std::unique_lock lock(mPlayerMutex);
        mPlayerMap[id.local] = plr;
    }

    spdlog::info("{} - New Player[{}] Login", __FUNCTION__, plr->GetFullID());
    co_await plr->OnLogin();

    // 同步玩家缓存数据
    SyncCache(plr);

    {
        std::unique_lock lock(mCacheMutex);
        mCacheMap[id.local].lastLoginTime = utils::UnixTime();
    }

    co_return plr;
}

void PlayerManager::OnPlayerLogout(const PlayerID pid) {
    spdlog::info("{} - Player[{}] Logout", __FUNCTION__, pid.ToInt64());
    if (const auto plr = RemovePlayer(pid.local); plr != nullptr) {
        plr->TryLeaveScene();
        plr->OnLogout();

        SyncCache(plr);

        std::unique_lock lock(mCacheMutex);
        mCacheMap[pid.local].lastLogoutTime = utils::UnixTime();
    }
}

std::shared_ptr<Player> PlayerManager::FindPlayer(const int32_t pid) {
    std::shared_lock lock(mPlayerMutex);
    if (const auto it = mPlayerMap.find(pid); it != mPlayerMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Player> PlayerManager::RemovePlayer(const int32_t pid) {
    std::unique_lock lock(mPlayerMutex);
    if (const auto it = mPlayerMap.find(pid); it != mPlayerMap.end()) {
        auto res = it->second;
        mPlayerMap.erase(it);
        return res;
    }
    return nullptr;
}

void PlayerManager::SendToList(const std::set<PlayerID> &players, const int32_t id, const std::string_view data) {
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

void PlayerManager::SyncCache(const std::shared_ptr<Player> &plr) {
    if (plr == nullptr)
        return;

    CacheNode cache{};
    {
        std::shared_lock lock(mCacheMutex);
        if (const auto iter = mCacheMap.find(plr->GetLocalID()); iter != mCacheMap.end()) {
            cache = iter->second;
        }
    }

    plr->SyncCache(&cache);
    SyncCache(cache);
}

void PlayerManager::SyncCache(const int32_t pid) {
    if (pid < PLAYER_LOCAL_ID_BEGIN || pid > PLAYER_LOCAL_ID_END)
        return;

    const auto plr = FindPlayer(pid);
    SyncCache(plr);
}

void PlayerManager::SyncCache(const CacheNode &node) {
    const PlayerID pid(node.pid);

    if (!pid.IsAvailable())
        return;

    std::unique_lock lock(mCacheMutex);
    mCacheMap[pid.local] = node;

    spdlog::trace("{} - Player[{}] Sync Success.", __FUNCTION__, pid.ToInt64());
}

awaitable<std::optional<CacheNode>> PlayerManager::FindCacheNode(const PlayerID &pid) {
    if (!pid.IsAvailable())
        co_return std::nullopt;

    if (pid.cross != GetWorld()->GetServerID()) {
        // TODO
        co_return std::nullopt;
    }

    if (const auto plr = FindPlayer(pid.local); plr != nullptr) {
        SyncCache(plr);
    }

    std::shared_lock lock(mCacheMutex);
    if (const auto it = mCacheMap.find(pid.local); it != mCacheMap.end()) {
        co_return std::make_optional(it->second);
    }

    co_return std::nullopt;
}

void PlayerManager::OnTick(const TimePoint now) {
    if (now - mLastUpdateTime < std::chrono::seconds(10))
        return;

    const auto ser = std::make_shared<Serializer>();
    auto *array = ser->CreateTableVector<orm::DBTable_PlayerCache>("player_cache");

    {

        std::shared_lock lock(mCacheMutex);
        for (const auto &val : mCacheMap | std::views::values) {
            orm::DBTable_PlayerCache table;
            table.pid = val.pid;
            table.cache << val;

            array->PushBack(table);
        }
    }

    if (const auto sys = GetWorld()->GetSystem<DatabaseSystem>(); sys != nullptr) {
        sys->PushTransaction([ser](mysqlx::Schema &schema) {
            ser->Serialize(schema);
        });
    }

    mLastUpdateTime = now;
    spdlog::trace("{} - Stored Cache.", __FUNCTION__);
}
