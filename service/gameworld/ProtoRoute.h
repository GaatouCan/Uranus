#pragma once

#include <functional>
#include <memory>
#include <absl/container/flat_hash_map.h>


class FPacket;
class UGameWorld;

class UProtoRoute final {

    using ARouteFunctor = std::function<void(uint32_t, const std::shared_ptr<FPacket> &, UGameWorld *)>;

public:
    UProtoRoute() = delete;

    explicit UProtoRoute(UGameWorld *world);
    ~UProtoRoute();

    void LoadProtocol();
    void Register(uint32_t id, const ARouteFunctor &func);

    void OnReceivePacket(const std::shared_ptr<FPacket> &packet);

private:
    UGameWorld *world_;
    absl::flat_hash_map<uint32_t, ARouteFunctor> routeMap_;
};

#define REGISTER_PROTOCOL(type, func) \
    Register(static_cast<uint32_t>(protocol::EProtoType::type), &protocol::func);
