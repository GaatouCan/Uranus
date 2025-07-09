#include "ProtoRoute.h"
#include "LoadProtocol.h"
#include "packet.h"

UProtoRoute::UProtoRoute()
    : mWorld(nullptr) {
    LoadProtocol();
}


void UProtoRoute::SetUpGameWorld(UGameWorld *world) {
    mWorld = world;
}

void UProtoRoute::OnReceivePacket(const std::shared_ptr<FPacket> &packet) const {
    OnReceivePackage(packet, mWorld);
}


