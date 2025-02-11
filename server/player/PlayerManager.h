#pragma once

#include "CacheNode.h"

#include "system/manager/BaseManager.h"
#include "PlayerID.h"

#include <mutex>
#include <shared_mutex>
#include <set>


class UPlayer;
class Connection;

class UPlayerManager final : public IBaseManager {

    std::unordered_map<uint32_t, std::shared_ptr<UPlayer>> mPlayerMap;
    std::mutex mPlayerMutex;
    mutable std::shared_mutex mPlayerSharedMutex;

    std::unordered_map<uint32_t, FCacheNode> mCacheMap;
    std::mutex mCacheMutex;
    mutable std::shared_mutex mCacheSharedMutex;

public:
    explicit UPlayerManager(ManagerSystem *owner);
    ~UPlayerManager() override;

    void Init() override;

    GET_MANAGER_NAME(UPlayerManager)

    void OnDayChange() override;

    awaitable<std::shared_ptr<UPlayer>> OnPlayerLogin(const std::shared_ptr<Connection> &conn, const FPlayerID &id);

    void OnPlayerLogout(FPlayerID pid);

    std::shared_ptr<UPlayer> FindPlayer(uint32_t pid);
    std::shared_ptr<UPlayer> RemovePlayer(uint32_t pid);

    void SendToList(const std::set<FPlayerID>& players, int32_t id, std::string_view data);

    void SyncCache(const std::shared_ptr<UPlayer> &plr);
    void SyncCache(uint32_t pid);
    void SyncCache(const FCacheNode &node);

    awaitable<std::optional<FCacheNode>> FindCacheNode(const FPlayerID &pid);
};

#define SEND_TO_PLAYER_SET(mgr, set, proto, data) \
    (mgr)->SendToList((set), static_cast<int32_t>(protocol::EProtoType::proto), (data).SerializeAsString());
