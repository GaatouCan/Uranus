#pragma once

#include "proto_route.h"
#include <proto_type.gen.h>

namespace protocol {
    void SyncPlayerInfo(uint32_t id, const std::shared_ptr<FPacket> &pkt, UGameWorld *world);
}

inline void UProtoRoute::LoadProtocol() {
    REGISTER_PROTOCOL(SYNC_PLAYER_INFO, SyncPlayerInfo)
}

