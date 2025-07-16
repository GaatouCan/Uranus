#include "PlayerManager.h"
#include "../../LoadProtocol.h"
#include "../../GameWorld.h"

#include <Packet.h>
#include <Utils.h>
#include <player.pb.h>

UPlayerManager::UPlayerManager(UGameWorld *world)
    : Super(world) {
}

UPlayerManager::~UPlayerManager() {
}

void UPlayerManager::OnSyncPlayer(const std::shared_ptr<FPacket> &pkt) {
    if (pkt == nullptr)
        return;

    Player::SyncPlayerInfo info;
    info.ParseFromString(pkt->ToString());

    FPlayerCache *cache = &cacheMap_[info.player_id()];
    if (cache->pid == 0) {
        cache->pid = info.player_id();
    }

    cache->name = info.name();
    cache->avatarIndex = info.avatar();

    cache->syncTime = utils::UnixTime();
}

const FPlayerCache *UPlayerManager::GetPlayerCache(const int64_t pid) const {
    const auto iter = cacheMap_.find(pid);
    return iter != cacheMap_.end() ? &iter->second : nullptr;
}

void protocol::SyncPlayerInfo(uint32_t id, const std::shared_ptr<FPacket> &pkt, UGameWorld *world) {
    auto *mgr = world->GetManager<UPlayerManager>();
    if (mgr == nullptr)
        return;

    mgr->OnSyncPlayer(pkt);
}

