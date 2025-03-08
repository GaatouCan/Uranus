#include "player_manager.h"

#include "player.h"
#include "../common/proto_type.h"

#include "game_world.h"
#include "connection.h"
#include "impl/package.h"
#include "config_manager.h"
#include "system/manager/manager_system.h"
#include "system/database/database_system.h"
#include "system/database/serializer.h"

#include "player_cache.orm.h"


static_assert(FByteArray::kPODType<FCacheNode>, "CacheNode Is Not POD Type");

UPlayerManager::UPlayerManager(UManagerSystem *owner)
    : IBaseManager(owner) {

}

UPlayerManager::~UPlayerManager() {
    playerMap_.clear();
}

void UPlayerManager::init() {
    if (const auto sys = getWorld()->getSystem<UDatabaseSystem>(); sys != nullptr) {
        sys->syncSelect("player_cache", "", [&](mysqlx::Row row) {
            orm::DBTable_PlayerCache table;
            table.read(row);

            const FPlayerID pid(table.pid);
            if (!pid.available())
                return;

            if (auto *node = &cacheMap_[pid.local]; node != nullptr)
                table.cache >> node;
        });
    }
    tickPerSecond_ = true;
}

void UPlayerManager::onDayChange() {
    for (const auto &plr: playerMap_ | std::views::values) {
        if (plr != nullptr && plr->isOnline()) {
            plr->onDayChange();
        }
    }
}

awaitable<std::shared_ptr<UPlayer>> UPlayerManager::onPlayerLogin(const std::shared_ptr<UConnection> &conn, const FPlayerID &id) {
    if (conn == nullptr || std::any_cast<FPlayerID>(conn->getContext()) != id) {
        spdlog::error("{} - Null Connection Pointer Or Player ID Not Equal.", __FUNCTION__);
        co_return nullptr;
    }

    if (const auto plr = findPlayer(id.local); plr != nullptr) {
        {
            std::scoped_lock lock(playerMutex_);
            playerMap_.erase(id.local);
        }

        spdlog::info("{} - Player[{}] Over Login", __FUNCTION__, plr->getFullID());

        plr->getConnection()->resetContext();
        plr->getConnection()->disconnect();

        plr->tryLeaveScene();

        if (plr->isOnline()) {
            plr->onLogout(true, conn->remoteAddress().to_string());
        }
    }

    const auto plr = std::make_shared<UPlayer>(conn);

    {
        std::unique_lock lock(playerMutex_);
        playerMap_[id.local] = plr;
    }

    spdlog::info("{} - New Player[{}] Login", __FUNCTION__, plr->getFullID());
    co_await plr->onLogin();

    // 同步玩家缓存数据
    syncCache(plr);
    co_return plr;
}

void UPlayerManager::onPlayerLogout(const FPlayerID pid) {
    spdlog::info("{} - Player[{}] Logout", __FUNCTION__, pid.toInt64());
    if (const auto plr = removePlayer(pid.local); plr != nullptr) {
        plr->tryLeaveScene();
        plr->onLogout();

        syncCache(plr);
    }
}

std::shared_ptr<UPlayer> UPlayerManager::findPlayer(const int32_t pid) {
    std::shared_lock lock(playerMutex_);
    if (const auto it = playerMap_.find(pid); it != playerMap_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<UPlayer> UPlayerManager::removePlayer(const int32_t pid) {
    std::unique_lock lock(playerMutex_);
    if (const auto it = playerMap_.find(pid); it != playerMap_.end()) {
        auto res = it->second;
        playerMap_.erase(it);
        return res;
    }
    return nullptr;
}

void UPlayerManager::sendToList(const std::set<FPlayerID> &players, const int32_t id, const std::string_view data) {
    if (id <= MINIMUM_PACKAGE_ID || id >= static_cast<int32_t>(protocol::EProtoType::PROTO_TYPE_MAX))
        return;

    for (const auto [localID, crossID]: players) {
        if (const auto plr = findPlayer(localID); plr != nullptr && plr->isOnline()) {
            const auto pkg = dynamic_cast<FPackage *>(plr->getConnection()->buildPackage());
            pkg->setPackageID(id).setData(data);
            plr->sendPackage(pkg);
        }
    }
}

void UPlayerManager::syncCache(const std::shared_ptr<UPlayer> &plr) {
    if (plr == nullptr)
        return;

    FCacheNode cache{};
    {
        std::shared_lock lock(cacheMutex_);
        if (const auto iter = cacheMap_.find(plr->getLocalID()); iter != cacheMap_.end()) {
            cache = iter->second;
        }
    }

    plr->syncCache(&cache);
    syncCache(cache);
}

void UPlayerManager::syncCache(const int32_t pid) {
    if (pid < PLAYER_LOCAL_ID_BEGIN || pid > PLAYER_LOCAL_ID_END)
        return;

    const auto plr = findPlayer(pid);
    syncCache(plr);
}

void UPlayerManager::syncCache(const FCacheNode &node) {
    const FPlayerID pid(node.pid);

    if (!pid.available())
        return;

    std::unique_lock lock(cacheMutex_);
    cacheMap_[pid.local] = node;

    spdlog::trace("{} - Player[{}] Sync Success.", __FUNCTION__, pid.toInt64());
}

awaitable<std::optional<FCacheNode>> UPlayerManager::findCacheNode(const FPlayerID &pid) {
    if (!pid.available())
        co_return std::nullopt;

    if (pid.cross != getWorld()->getServerID()) {
        // TODO
        co_return std::nullopt;
    }

    if (const auto plr = findPlayer(pid.local); plr != nullptr) {
        syncCache(plr);
    }

    std::shared_lock lock(cacheMutex_);
    if (const auto it = cacheMap_.find(pid.local); it != cacheMap_.end()) {
        co_return std::make_optional(it->second);
    }

    co_return std::nullopt;
}

void UPlayerManager::onTick(const ATimePoint now, const ADuration interval) {
    if (now - lastUpdateTime_ < std::chrono::seconds(10))
        return;

    const auto ser = std::make_shared<USerializer>();

    {
        auto *array = ser->createTableVector<orm::DBTable_PlayerCache>("player_cache");

        std::shared_lock lock(cacheMutex_);
        for (const auto &val : cacheMap_ | std::views::values) {
            orm::DBTable_PlayerCache table;
            table.pid = val.pid;
            table.cache << val;

            array->pushBack(table);
        }
    }

    if (const auto sys = getWorld()->getSystem<UDatabaseSystem>(); sys != nullptr) {
        sys->pushTransaction([ser](mysqlx::Schema &schema) {
            ser->serialize(schema);
        });
    }

    lastUpdateTime_ = now;
    spdlog::trace("{} - Stored Cache.", __FUNCTION__);
}
