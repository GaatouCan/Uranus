#pragma once

#include "cache_node.h"

#include "system/manager/base_manager.h"
#include "player_id.h"

#include <shared_mutex>
#include <set>


class UPlayer;
class UConnection;

class PlayerManager final : public IBaseManager {

    std::unordered_map<int32_t, std::shared_ptr<UPlayer>> player_map_;
    mutable std::shared_mutex player_mtx_;

    std::unordered_map<int32_t, CacheNode> cache_map_;
    mutable std::shared_mutex cache_mtx_;

    ATimePoint last_update_time_;

public:
    explicit PlayerManager(UManagerSystem *owner);
    ~PlayerManager() override;

    void init() override;

    GET_MANAGER_NAME(PlayerManager)

    void onDayChange() override;

    awaitable<std::shared_ptr<UPlayer>> OnPlayerLogin(const std::shared_ptr<UConnection> &conn, const FPlayerID &id);

    void OnPlayerLogout(FPlayerID pid);

    std::shared_ptr<UPlayer> FindPlayer(int32_t pid);
    std::shared_ptr<UPlayer> RemovePlayer(int32_t pid);

    void SendToList(const std::set<FPlayerID>& players, int32_t id, std::string_view data);

    void SyncCache(const std::shared_ptr<UPlayer> &plr);
    void SyncCache(int32_t pid);
    void SyncCache(const CacheNode &node);

    awaitable<std::optional<CacheNode>> FindCacheNode(const FPlayerID &pid);

    void onTick(ATimePoint now, ADuration interval) override;
};

#define SEND_TO_PLAYER_SET(mgr, set, proto, data) \
    (mgr)->SendToList((set), static_cast<int32_t>(protocol::EProtoType::proto), (data).SerializeAsString());
