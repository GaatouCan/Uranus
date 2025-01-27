#include "AppearanceCT.h"

#include "../../../player/ComponentModule.h"
#include "../../../player/Player.h"
#include "../../../common/proto.def.h"

#include <impl/Package.h>
#include <utils.h>
#include <GameWorld.h>
#include <system/config/ConfigSystem.h>

#include <ranges>
#include <utility>
#include <spdlog/spdlog.h>
#include <appearance.pb.h>


UAppearanceCT::UAppearanceCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {

    SERIALIZE_COMPONENT(UAppearanceCT, Appearance)
    SERIALIZE_COMPONENT(UAppearanceCT, Avatar)
    SERIALIZE_COMPONENT(UAppearanceCT, AvatarFrame)
}

ISerializer *UAppearanceCT::Serialize_Appearance(bool &bExpired) const {
    READ_PARAM(Appearance, mAppearance)
}

void UAppearanceCT::Deserialize_Appearance(UDeserializer &ds) {
    WRITE_PARAM(ds, mAppearance)
}

ISerializer *UAppearanceCT::Serialize_Avatar(bool &bExpired) const {
    READ_PARAM_MAP(Avatar, mAvatarMap)
}

void UAppearanceCT::Deserialize_Avatar(UDeserializer &ds) {
    WRITE_PARAM_MAP(ds, mAvatarMap)
}

ISerializer *UAppearanceCT::Serialize_AvatarFrame(bool &bExpired) const {
    READ_PARAM_MAP(AvatarFrame, mAvatarFrameMap)
}

void UAppearanceCT::Deserialize_AvatarFrame(UDeserializer &ds) {
    WRITE_PARAM_MAP(ds, mAvatarFrameMap)
}

void UAppearanceCT::SendInfo() const {
    Appearance::W2C_AppearanceResponse res;

    res.set_current_avatar(mAppearance.avatar);
    res.set_current_avatar_frame(mAppearance.avatar_frame);

    for (auto &val: mAvatarMap | std::views::values) {
        const auto avatar = res.add_avatar();
        avatar->set_index(val.index);
        avatar->set_bactivated(val.activated);
        avatar->set_expired(val.expired_time);
    }

    for (auto &val: mAvatarFrameMap | std::views::values) {
        const auto frame = res.add_avatar_frame();
        frame->set_index(val.index);
        frame->set_bactivated(val.activated);
        frame->set_expired(val.expired_time);
    }

    SEND_PACKAGE(this, W2C_AppearanceResponse, res)
}

void UAppearanceCT::ActiveAvatar(const int index) {
    if (const auto it = mAvatarMap.find(index); it != mAvatarMap.end()) {
        if (it->second.activated) {
            SendInfo();
            return;
        }
    }

    const auto cfg = UConfigSystem::Instance()->FindConfig("appearance.avatar", index);
    if (!cfg.has_value())
        return;

    if (!mAvatarMap.contains(index)) {
        mAvatarMap[index] = orm::UDBTable_Avatar();
    }

    mAvatarMap[index].activated = true;
    if (const auto expired = (*cfg)["expired"].get<uint64_t>(); expired > 0) {
        mAvatarMap[index].expired_time = utils::UnixTime() + expired;
    }

    mAppearance.update_time = utils::UnixTime();
    SendInfo();
}

awaitable<void> protocol::C2W_AppearanceRequest(const std::shared_ptr<IBasePlayer> &plr, IPackage *pkg) {
    if (plr == nullptr)
        co_return;

    const auto player = std::dynamic_pointer_cast<UPlayer>(plr);
    const auto package = dynamic_cast<FPackage *>(pkg);

    Appearance::C2W_AppearanceRequest req;
    req.ParseFromArray(&package->GetByteArray(), package->GetDataLength());

    const auto ct = player->GetComponentModule().GetComponent<UAppearanceCT>();
    if (ct == nullptr)
        co_return;

    switch (req.operate_type()) {
        case Appearance::SEND_INFO: {
            ct->SendInfo();
        } break;
        default: break;
    }
}
