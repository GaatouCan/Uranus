#pragma once


#include <Service/ProtocolRoute.h>
#include <Packet.h>

#include <functional>


class UPlayer;

using AProtoFunctor = std::function<void(uint32_t, const std::shared_ptr<FPacket> &, UPlayer *)>;

class UProtoRoute final : public TProtocolRoute<FPacket, AProtoFunctor>{

    UPlayer *mPlayer;

public:
    UProtoRoute();

    void SetUpPlayer(UPlayer *owner);

    void LoadProtocol();
    void OnReceivePacket(const std::shared_ptr<FPacket> &pkt) const;
};

#define REGISTER_PROTOCOL(type, func) \
    Register(static_cast<uint32_t>(protocol::EProtoType::type), &protocol::func);
