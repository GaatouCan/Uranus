#include "Appear.h"
#include "../../LoadProtocol.h"
#include "../../Player.h"
#include <Packet.h>

#include <ProtoType.gen.h>
#include <appearance.pb.h>


UAppear::UAppear() {
}

void UAppear::ActiveAvatar(const int index) {
    if (const auto iter = avatarMap_.find(index); iter != avatarMap_.end()) {
        if (iter->second.activated)
            return;
    }

    auto &info = avatarMap_[index];

    info.pid = GetPlayerID();
    info.index = index;
    info.expired_time = 0;
    info.activated = true;

    appear_.avatar = index;

    SendInfo();
}

void UAppear::UseAvatar(const int index) {
    const auto iter = avatarMap_.find(index);
    if (iter == avatarMap_.end())
        return;

    if (!iter->second.activated)
        return;

    if (appear_.avatar == index)
        return;

    appear_.avatar = index;
    SendInfo();
}

void UAppear::SendInfo() const {
    Appearance::AppearanceResponse response;

    response.set_avatar(1);

    for (const auto &val: avatarMap_ | std::views::values) {
        auto *avatar = response.add_list();
        avatar->set_index(val.index);
        avatar->set_active(val.activated);
        avatar->set_in_used(val.index == appear_.avatar);
    }

    SEND_TO_CLIENT(this, APPEARANCE_RESPONSE, response)
}

void protocol::AppearanceRequest(uint32_t id, const std::shared_ptr<FPacket> &pkt, UPlayer *plr) {
    if (pkt == nullptr)
        return;

    if (plr == nullptr)
        return;

    const auto appearCT = plr->GetComponent<UAppear>();
    if (appearCT == nullptr)
        return;

    Appearance::AppearanceRequest request;
    request.ParseFromString(pkt->ToString());

    switch (request.operate()) {
        case Appearance::AppearanceRequest::SEND_INFO: {
            appearCT->SendInfo();
        }
        break;
        case Appearance::AppearanceRequest::ACTIVE_AVATAR: {
            appearCT->ActiveAvatar(request.param_1());
        }
        break;
        case Appearance::AppearanceRequest::USE_AVATAR: {
            appearCT->UseAvatar(request.param_1());
        }
        break;
        default:
            break;
    }
}
