//
// Created by admin on 25-2-12.
//

#include "appearance_ct.h"

#include "../../../common/proto.def.h"
#include "../../../player/component_module.h"
#include "../../../player/player.h"
#include "../../../player/cache_node.h"

#include "impl/package.h"
#include "utils.h"
#include "game_world.h"
#include "config_manager.h"

#include <appearance.pb.h>


AppearanceCT::AppearanceCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {
    COMPONENT_TABLE(AppearanceCT, Appearance)
    COMPONENT_TABLE(AppearanceCT, Avatar)
    COMPONENT_TABLE(AppearanceCT, AvatarFrame)
}

AppearanceCT::~AppearanceCT() {
}

ISerializer *AppearanceCT::Serialize_Appearance(bool &bExpired) const {
    WRITE_TABLE(Appearance, mAppear)
}

void AppearanceCT::Deserialize_Appearance(Deserializer &ds) {
    READ_TABLE(ds, mAppear)
}

ISerializer *AppearanceCT::Serialize_Avatar(bool &bExpired) const {
    WRITE_TABLE_MAP(Avatar, mAvatarMap)
}

void AppearanceCT::Deserialize_Avatar(Deserializer &ds) {
    READ_TABLE_MAP(ds, mAvatarMap)
}

ISerializer *AppearanceCT::Serialize_AvatarFrame(bool &bExpired) const {
    WRITE_TABLE_MAP(AvatarFrame, mAvatarFrameMap)
}

void AppearanceCT::Deserialize_AvatarFrame(Deserializer &ds) {
    READ_TABLE_MAP(ds, mAvatarFrameMap)
}

void AppearanceCT::OnLogin() {
    if (mAppear.pid == 0)
        mAppear.pid = GetOwner()->GetFullID();
}

void AppearanceCT::SendInfo() const {
    Appearance::AppearanceResponse res;

    res.set_current_avatar(mAppear.avatar);
    res.set_current_avatar_frame(mAppear.avatar_frame);

    for (const auto &val: mAvatarMap | std::views::values) {
        const auto avatar = res.add_avatar();
        avatar->set_index(val.index);
        avatar->set_bactivated(val.activated);
        avatar->set_expired(val.expired_time);
    }

    for (const auto &val: mAvatarFrameMap | std::views::values) {
        const auto avatar_frame = res.add_avatar_frame();
        avatar_frame->set_index(val.index);
        avatar_frame->set_bactivated(val.activated);
        avatar_frame->set_expired(val.expired_time);
    }

    SEND_PACKAGE(this, AppearanceResponse, res);
}

void AppearanceCT::ActiveAvatar(const int index, const bool bAutoUse) {
    CheckAvatar(index);

    auto iter = mAvatarMap.find(index);
    if (iter != mAvatarMap.end() && iter->second.activated)
        return;

    const auto cfg_op = GetWorld()->GetConfigManager()->FindConfig("appearance.avatar", index);
    if (!cfg_op.has_value())
        return;

    // const auto cfg = cfg_op.value();
    orm::DBTable_Avatar avatar;

    avatar.pid = GetOwner()->GetFullID();
    avatar.index = index;
    avatar.activated = true;
    avatar.expired_time = utils::UnixTime() + 1000000;

    mAvatarMap[index] = avatar;
    iter = mAvatarMap.find(index);

    if (bAutoUse) {
        // iter->second.in_used = true;
        mAppear.avatar = index;
    }

    SendInfo();
}

void AppearanceCT::UseAvatar(const int index) {
    CheckAvatar(index);

    const auto iter = mAvatarMap.find(index);
    if (iter == mAvatarMap.end() || !iter->second.activated)
        return;

    mAppear.avatar = index;

    SendInfo();
}

void AppearanceCT::ActiveAvatarFrame(const int index, bool bAutoUse) {
    CheckAvatarFrame(index);

    auto iter = mAvatarFrameMap.find(index);
    if (iter != mAvatarFrameMap.end() && iter->second.activated)
        return;

    const auto cfg_op = GetWorld()->GetConfigManager()->FindConfig("appearance.avatar", index);
    if (!cfg_op.has_value())
        return;

    // const auto cfg = cfg_op.value();
    orm::DBTable_AvatarFrame frame;

    frame.pid = GetOwner()->GetFullID();
    frame.index = index;
    frame.activated = true;
    frame.expired_time = utils::UnixTime() + 1000000;

    mAvatarFrameMap[index] = frame;
    iter = mAvatarFrameMap.find(index);

    if (bAutoUse) {
        // iter->second.in_used = true;
        mAppear.avatar_frame = index;
    }

    SendInfo();
}

void AppearanceCT::UseAvatarFrame(const int index) {
    CheckAvatarFrame(index);

    const auto iter = mAvatarFrameMap.find(index);
    if (iter == mAvatarFrameMap.end() || !iter->second.activated)
        return;

    mAppear.avatar_frame = index;

    SendInfo();
}

void AppearanceCT::SyncCache(CacheNode* node) {
    node->avatar = mAppear.avatar;
    node->avatarFrame = mAppear.avatar_frame;
}

void AppearanceCT::CheckAvatar(const int index) {
    const auto iter = mAvatarMap.find(index);
    if (iter == mAvatarMap.end())
        return;

    const auto elem = &iter->second;
    if (!elem->activated)
        return;

    if (elem->expired_time < 0)
        return;

    if (elem->expired_time >= 0 && elem->expired_time < utils::UnixTime())
        elem->activated = false;
}

void AppearanceCT::CheckAvatarFrame(const int index) {
    const auto iter = mAvatarFrameMap.find(index);
    if (iter == mAvatarFrameMap.end())
        return;

    const auto elem = &iter->second;
    if (!elem->activated)
        return;

    if (elem->expired_time < 0)
        return;

    if (elem->expired_time >= 0 && elem->expired_time < utils::UnixTime())
        elem->activated = false;
}

void protocol::AppearanceRequest(const std::shared_ptr<IBasePlayer> &plr, IPackage *pkg) {
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
        }
        break;
        case Appearance::ACTIVE_AVATAR: {
            ct->ActiveAvatar(request.parameter(), request.extend() == 1);
        }
        break;
        case Appearance::USE_AVATAR: {
            ct->UseAvatar(request.parameter());
        }
        break;
        case Appearance::ACTIVE_AVATAR_FRAME: {
            ct->ActiveAvatarFrame(request.parameter(), request.extend() == 1);
        }
        break;
        case Appearance::USE_AVATAR_FRAME: {
            ct->UseAvatarFrame(request.parameter());
        }
        break;
        default:
            break;
    }
}
