#include "LoginHandlerImpl.h"
#include "Internal/Packet.h"

#include <Server.h>
#include <Service/ServiceModule.h>

#include <login.pb.h>

ULoginHandler::ULoginHandler(ULoginAuth *module)
    : ILoginHandler(module) {
}

ULoginHandler::~ULoginHandler() {
}

void ULoginHandler::UpdateAddressList() {
}

ILoginHandler::FLoginToken ULoginHandler::ParseLoginRequest(const std::shared_ptr<IPackageBase> &pkg) {
    const auto pkt = std::dynamic_pointer_cast<FPacket>(pkg);
    if (pkt == nullptr)
        return {};

    if (pkt->GetID() != 1002)
        return {};

    Login::LoginRequest request;
    request.ParseFromString(pkt->ToString());

    return {
        request.token(),
        request.player_id()
    };
}

void ULoginHandler::OnLoginSuccess(const int64_t pid, const std::shared_ptr<IPackageBase> &pkg) const {
    const auto *service = GetServer()->GetModule<UServiceModule>();
    if (service == nullptr)
        return;

    const auto pkt = std::dynamic_pointer_cast<FPacket>(pkg);
    if (pkt == nullptr)
        return;

    Login::LoginResponse res;

    res.set_player_id(pid);

    for (const auto &[name, id]: service->GetServiceList()) {
        auto *val = res.add_services();
        val->set_name(name);
        val->set_sid(id);
    }

    pkt->SetPackageID(1003);
    pkt->SetData(res.SerializeAsString());
}

void ULoginHandler::OnRepeatLogin(const int64_t pid, const std::string &addr, const std::shared_ptr<IPackageBase> &pkg) {
    const auto pkt = std::dynamic_pointer_cast<FPacket>(pkg);
    if (pkt == nullptr)
        return;

    Login::RepeatLoginResponse response;

    response.set_player_id(pid);
    response.set_address(addr);

    pkt->SetPackageID(1004);
    pkt->SetData(response.SerializeAsString());
}
