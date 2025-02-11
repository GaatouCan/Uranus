#pragma once

#include "CacheNode.h"

#include "system/manager/BaseManager.h"
#include "PlayerID.h"

#include <mutex>
#include <shared_mutex>
#include <set>


class Player;
class Connection;

class UPlayerManager final : public IBaseManager {

    std::unordered_map<uint32_t, std::shared_ptr<Player>> mPlayerMap;
    std::mutex mPlayerMutex;
    mutable std::shared_mutex mPlayerSharedMutex;

    std::unordered_map<uint32_t, CacheNode> mCacheMap;
    std::mutex mCacheMutex;
    mutable std::shared_mutex mCacheSharedMutex;

public:
    explicit UPlayerManager(ManagerSystem *owner);
    ~UPlayerManager() override;

    void Init() override;

    GET_MANAGER_NAME(UPlayerManager)

    void OnDayChange() override;

    awaitable<std::shared_ptr<Player>> OnPlayerLogin(const std::shared_ptr<Connection> &conn, const PlayerID &id);

    void OnPlayerLogout(PlayerID pid);

    std::shared_ptr<Player> FindPlayer(uint32_t pid);
    std::shared_ptr<Player> RemovePlayer(uint32_t pid);

    void SendToList(const std::set<PlayerID>& players, int32_t id, std::string_view data);

    void SyncCache(const std::shared_ptr<Player> &plr);
    void SyncCache(uint32_t pid);
    void SyncCache(const CacheNode &node);

    awaitable<std::optional<CacheNode>> FindCacheNode(const PlayerID &pid);
};

#define SEND_TO_PLAYER_SET(mgr, set, proto, data) \
    (mgr)->SendToList((set), static_cast<int32_t>(protocol::EProtoType::proto), (data).SerializeAsString());
