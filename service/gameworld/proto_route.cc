#include "proto_route.h"
#include "load_protocol.h"
#include "packet.h"

UProtoRoute::UProtoRoute(UGameWorld *world)
    : world_(world) {
    LoadProtocol();
}

UProtoRoute::~UProtoRoute() {

}

void UProtoRoute::Register(const uint32_t id, const ARouteFunctor &func) {
    routeMap_.emplace(id, func);
}

void UProtoRoute::OnReceivePacket(const std::shared_ptr<FPacket> &packet) {
    if (packet == nullptr)
        return;

    const auto id = packet->GetID();

    if (const auto iter = routeMap_.find(id); iter != routeMap_.end()) {
        std::invoke(iter->second, id, packet, world_);
    }
}


