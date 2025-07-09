#pragma once

#include "ProtoRoute.h"
#include <ProtoType.gen.h>

namespace protocol {
    void SyncPlayerInfo(uint32_t id, const std::shared_ptr<FPacket> &pkt, UGameWorld *world);
}

inline void UProtoRoute::LoadProtocol() {
    REGISTER_PROTOCOL(SYNC_PLAYER_INFO, SyncPlayerInfo)
}

