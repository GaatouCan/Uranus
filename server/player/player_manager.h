#pragma once

#include "cache_node.h"

#include "system/manager/base_manager.h"
#include "player_id.h"

#include <shared_mutex>
#include <set>


class UPlayer;
class UConnection;

class UPlayerManager final : public IBaseManager {

    std::unordered_map<int32_t, std::shared_ptr<UPlayer>> playerMap_;
    mutable std::shared_mutex playerMutex_;

    std::unordered_map<int32_t, FCacheNode> cacheMap_;
    mutable std::shared_mutex cacheMutex_;

    ATimePoint lastUpdateTime_;

public:
    explicit UPlayerManager(UManagerSystem *owner);
    ~UPlayerManager() override;

    void init() override;

    GET_MANAGER_NAME(PlayerManager)

    void onDayChange() override;

    awaitable<std::shared_ptr<UPlayer>> onPlayerLogin(const std::shared_ptr<UConnection> &conn, const FPlayerID &id);

    void onPlayerLogout(FPlayerID pid);

    std::shared_ptr<UPlayer> findPlayer(int32_t pid);
    std::shared_ptr<UPlayer> removePlayer(int32_t pid);

    void sendToList(const std::set<FPlayerID>& players, int32_t id, std::string_view data);

    void syncCache(const std::shared_ptr<UPlayer> &plr);
    void syncCache(int32_t pid);
    void syncCache(const FCacheNode &node);

    awaitable<std::optional<FCacheNode>> findCacheNode(const FPlayerID &pid);

    void onTick(ATimePoint now, ADuration interval) override;
};

#define SEND_TO_PLAYER_SET(mgr, set, proto, data) \
    (mgr)->sendToList((set), static_cast<int32_t>(protocol::EProtoType::proto), (data).SerializeAsString());
