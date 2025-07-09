#include "Player.h"
#include "ProtoRoute.h"

#include <Config/Config.h>
#include <Packet.h>

#include <player.pb.h>

UPlayer::UPlayer()
    : mComponent(this) {
    mRoute.SetUpPlayer(this);
}

UPlayer::~UPlayer() {
}

bool UPlayer::Initial(const std::shared_ptr<IPackage> &pkg) {
    if (!Super::Initial(pkg))
        return false;

    const auto res = mConfig.LoadConfig(GetModule<UConfig>());
    if (res != 0)
        return false;

    return true;
}

bool UPlayer::Start() {
    Super::Start();

    Player::SyncPlayerInfo info;

    info.set_player_id(GetPlayerID());
    info.set_name("Player");

    SendToService("Game World", 1301, info.SerializeAsString());
    return true;
}

void UPlayer::Stop() {
    Super::Stop();
}

void UPlayer::OnPackage(const std::shared_ptr<IPackage> &pkg) {
    if (const auto pkt = std::dynamic_pointer_cast<FPacket>(pkg)) {
        mRoute.OnReceivePacket(pkt);
    }
}

void UPlayer::SendToClient(const uint32_t id, const std::string &data) const {
    if (const auto pkt = std::dynamic_pointer_cast<FPacket>(BuildPackage())) {
        pkt->SetID(id);
        pkt->SetData(data);
        Super::SendToClient(pkt);
    }
}

void UPlayer::SendToService(const int32_t sid, const uint32_t id, const std::string &data) const {
    if (const auto pkt = std::dynamic_pointer_cast<FPacket>(BuildPackage())) {
        pkt->SetTarget(sid);
        pkt->SetID(id);
        pkt->SetData(data);
        PostPackage(pkt);
    }
}

void UPlayer::SendToService(const std::string &name, const uint32_t id, const std::string &data) const {
    if (const auto pkt = std::dynamic_pointer_cast<FPacket>(BuildPackage())) {
        pkt->SetID(id);
        pkt->SetData(data);
        PostPackage(name, pkt);
    }
}

void UPlayer::SendToPlayer(const int64_t pid, const uint32_t id, const std::string &data) const {
    if (const auto pkt = std::dynamic_pointer_cast<FPacket>(BuildPackage())) {
        pkt->SetID(id);
        pkt->SetData(data);
        Super::SendToPlayer(pid, pkt);
    }
}

UComponentModule &UPlayer::GetComponentModule() {
    return mComponent;
}

extern "C" SERVICE_API IPlayerAgent *NewService() {
    return new UPlayer();
}

extern "C" SERVICE_API void DestroyService(IService *service) {
    delete service;
}
