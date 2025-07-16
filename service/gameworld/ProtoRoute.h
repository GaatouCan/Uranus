#pragma once

#include <functional>
#include <memory>
#include <absl/container/flat_hash_map.h>

#include <service/ProtocolRoute.h>
#include <Packet.h>



class UGameWorld;

using ARouteFunctor = std::function<void(uint32_t, const std::shared_ptr<FPacket> &, UGameWorld *)>;

class UProtoRoute final : public TProtocolRoute<FPacket, ARouteFunctor> {

public:
    UProtoRoute();

    void SetUpGameWorld(UGameWorld *world);

    void LoadProtocol();
    void OnReceivePacket(const std::shared_ptr<FPacket> &packet) const;

private:
    UGameWorld *mWorld;
};

#define REGISTER_PROTOCOL(type, func) \
    Register(static_cast<uint32_t>(protocol::EProtoType::type), &protocol::func);
