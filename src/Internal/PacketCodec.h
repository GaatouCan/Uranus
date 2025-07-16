#pragma once

#include "Packet.h"
#include "Network/PackageCodec.h"


class BASE_API UPacketCodec final : public TPackageCodec<FPacket> {

public:
    explicit UPacketCodec(ATcpSocket &socket);

    awaitable<bool> EncodeT(const std::shared_ptr<FPacket> &pkt) override;
    awaitable<bool> DecodeT(const std::shared_ptr<FPacket> &pkt) override;
};
