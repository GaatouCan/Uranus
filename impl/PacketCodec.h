#pragma once

#include "Packet.h"
#include <Network/PackageCodec.h>


class IMPL_API UPacketCodec final : public TPackageCodec<FPacket> {

    friend class UConnection;

protected:
    explicit UPacketCodec(ATcpSocket &socket);

public:
    awaitable<bool> EncodeT(const std::shared_ptr<FPacket> &pkt) override;
    awaitable<bool> DecodeT(const std::shared_ptr<FPacket> &pkt) override;
};
