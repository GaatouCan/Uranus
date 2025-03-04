#pragma once

#include "cache_node.h"

#include "system/manager/base_manager.h"
#include "player_id.h"

#include <shared_mutex>
#include <set>


class Player;
class Connection;

class PlayerManager final : public IBaseManager {

    std::unordered_map<int32_t, std::shared_ptr<Player>> player_map_;
    mutable std::shared_mutex player_mtx_;

    std::unordered_map<int32_t, CacheNode> cache_map_;
    mutable std::shared_mutex cache_mtx_;

    TimePoint last_update_time_;

public:
    explicit PlayerManager(ManagerSystem *owner);
    ~PlayerManager() override;

    void Init() override;

    GET_MANAGER_NAME(PlayerManager)

    void OnDayChange() override;

    awaitable<std::shared_ptr<Player>> OnPlayerLogin(const std::shared_ptr<Connection> &conn, const PlayerID &id);

    void OnPlayerLogout(PlayerID pid);

    std::shared_ptr<Player> FindPlayer(int32_t pid);
    std::shared_ptr<Player> RemovePlayer(int32_t pid);

    void SendToList(const std::set<PlayerID>& players, int32_t id, std::string_view data);

    void SyncCache(const std::shared_ptr<Player> &plr);
    void SyncCache(int32_t pid);
    void SyncCache(const CacheNode &node);

    awaitable<std::optional<CacheNode>> FindCacheNode(const PlayerID &pid);

    void OnTick(TimePoint now, Duration interval) override;
};

#define SEND_TO_PLAYER_SET(mgr, set, proto, data) \
    (mgr)->SendToList((set), static_cast<int32_t>(protocol::EProtoType::proto), (data).SerializeAsString());
