#pragma once

#include "ProtoRoute.h"

#include <ProtoType.gen.h>

namespace protocol {
    void AppearanceRequest(uint32_t, const std::shared_ptr<FPacket> &, UPlayer *);
}

inline void UProtoRoute::LoadProtocol() {
    REGISTER_PROTOCOL(APPEARANCE_REQUEST, AppearanceRequest)
}
