//
// Created by admin on 25-2-12.
//

#include "appearance_ct.h"

#include "../../../common/proto.def.h"
#include "utils.h"

#include "../../../player/component_module.h"
#include "../../../player/player.h"

#include "impl/package.h"

#include <appearance.pb.h>

AppearanceCT::AppearanceCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {

    SERIALIZE_COMPONENT(AppearanceCT, Appearance)
    SERIALIZE_COMPONENT(AppearanceCT, Avatar)
    SERIALIZE_COMPONENT(AppearanceCT, AvatarFrame)
}

AppearanceCT::~AppearanceCT()
{

}

ISerializer *AppearanceCT::Serialize_Appearance(bool &bExpired) const
{
    READ_PARAM(Appearance, mAppear)
}

void AppearanceCT::Deserialize_Appearance(Deserializer &ds)
{
    WRITE_PARAM(ds, mAppear)
}

ISerializer *AppearanceCT::Serialize_Avatar(bool &bExpired) const
{
    READ_PARAM_VECTOR(Avatar, mAvatarList)
}

void AppearanceCT::Deserialize_Avatar(Deserializer &ds)
{
    WRITE_PARAM_VECTOR(ds, mAvatarList)
}

ISerializer *AppearanceCT::Serialize_AvatarFrame(bool &bExpired) const
{
    READ_PARAM_VECTOR(AvatarFrame, mAvatarFrameList)
}

void AppearanceCT::Deserialize_AvatarFrame(Deserializer &ds)
{
    WRITE_PARAM_VECTOR(ds, mAvatarFrameList)
}

void AppearanceCT::SendInfo() const
{
    Appearance::AppearanceResponse res;

    res.set_current_avatar(mAppear.avatar);
    res.set_current_avatar_frame(mAppear.avatar_frame);

    for (const auto &val : mAvatarList) {
        const auto avatar = res.add_avatar();
        avatar->set_index(val.index);
        avatar->set_bactivated(val.activated);
        avatar->set_expired(val.expired_time);
    }

    for (const auto &val : mAvatarFrameList) {
        const auto avatar_frame = res.add_avatar_frame();
        avatar_frame->set_index(val.index);
        avatar_frame->set_bactivated(val.activated);
        avatar_frame->set_expired(val.expired_time);
    }

    SEND_PACKAGE(this, AppearanceResponse, res);
}

void protocol::AppearanceRequest(const std::shared_ptr<IBasePlayer> &plr, IPackage *pkg)
{
    if (plr == nullptr)
        return;

    const auto ct = std::dynamic_pointer_cast<Player>(plr)->GetComponent<AppearanceCT>();
    if (ct == nullptr)
        return;

    Appearance::AppearanceRequest request;
    request.ParseFromString(dynamic_cast<Package *>(pkg)->ToString());

    switch (request.operate_type()) {
    case Appearance::SEND_INFO: {
        ct->SendInfo();
    } break;
    default:
        break;
    }
}
