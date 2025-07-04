#include "ProtoRoute.h"

UProtoRoute::UProtoRoute()
    : mPlayer(nullptr) {
    LoadProtocol();
}

void UProtoRoute::SetUpPlayer(UPlayer *owner) {
    mPlayer = owner;
}

void UProtoRoute::OnReceivePacket(const std::shared_ptr<FPacket> &pkt) const {
    OnReceivePackage(pkt, mPlayer);
}
