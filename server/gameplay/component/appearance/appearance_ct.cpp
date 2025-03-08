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

#include <system/database/serializer.h>
#include <system/database/deserializer.h>

#include <appearance.pb.h>


AppearanceCT::AppearanceCT(ComponentModule *module)
    : IPlayerComponent(module) {

}

AppearanceCT::~AppearanceCT() {
}

void AppearanceCT::Serialize(const std::shared_ptr<USerializer> &s) {
    WRITE_TABLE(s, Appearance, appear_)
    WRITE_TABLE_MAP(s, Avatar, avatar_map_)
    WRITE_TABLE_MAP(s, AvatarFrame, avatar_frame_map_)
}

void AppearanceCT::Deserialize(UDeserializer &ds) {
    READ_TABLE(&ds, Appearance, appear_)
    READ_TABLE_MAP(&ds, Avatar, avatar_map_)
    READ_TABLE_MAP(&ds, AvatarFrame, avatar_frame_map_)
}

void AppearanceCT::OnLogin() {
    if (appear_.pid == 0)
        appear_.pid = GetOwner()->getFullID();
}

void AppearanceCT::SendInfo() const {
    Appearance::AppearanceResponse res;

    res.set_current_avatar(appear_.avatar);
    res.set_current_avatar_frame(appear_.avatar_frame);

    for (const auto &val: avatar_map_ | std::views::values) {
        const auto avatar = res.add_avatar();
        avatar->set_index(val.index);
        avatar->set_bactivated(val.activated);
        avatar->set_expired(val.expired_time);
    }

    for (const auto &val: avatar_frame_map_ | std::views::values) {
        const auto avatar_frame = res.add_avatar_frame();
        avatar_frame->set_index(val.index);
        avatar_frame->set_bactivated(val.activated);
        avatar_frame->set_expired(val.expired_time);
    }

    SEND_PACKAGE(this, AppearanceResponse, res);
}

void AppearanceCT::ActiveAvatar(const int index, const bool bAutoUse) {
    CheckAvatar(index);

    auto iter = avatar_map_.find(index);
    if (iter != avatar_map_.end() && iter->second.activated)
        return;

    const auto cfg_op = GetWorld()->getConfigManager()->find("appearance.avatar", index);
    if (!cfg_op.has_value())
        return;

    // const auto cfg = cfg_op.value();
    orm::DBTable_Avatar avatar;

    avatar.pid = GetOwner()->getFullID();
    avatar.index = index;
    avatar.activated = true;
    avatar.expired_time = utils::UnixTime() + 1000000;

    avatar_map_[index] = avatar;
    iter = avatar_map_.find(index);

    if (bAutoUse) {
        // iter->second.in_used = true;
        appear_.avatar = index;
    }
}

void AppearanceCT::UseAvatar(const int index) {
    CheckAvatar(index);

    const auto iter = avatar_map_.find(index);
    if (iter == avatar_map_.end() || !iter->second.activated)
        return;

    appear_.avatar = index;
}

void AppearanceCT::ActiveAvatarFrame(const int index, bool bAutoUse) {
    CheckAvatarFrame(index);

    auto iter = avatar_frame_map_.find(index);
    if (iter != avatar_frame_map_.end() && iter->second.activated)
        return;

    const auto cfg_op = GetWorld()->getConfigManager()->find("appearance.avatar", index);
    if (!cfg_op.has_value())
        return;

    // const auto cfg = cfg_op.value();
    orm::DBTable_AvatarFrame frame;

    frame.pid = GetOwner()->getFullID();
    frame.index = index;
    frame.activated = true;
    frame.expired_time = utils::UnixTime() + 1000000;

    avatar_frame_map_[index] = frame;
    iter = avatar_frame_map_.find(index);

    if (bAutoUse) {
        // iter->second.in_used = true;
        appear_.avatar_frame = index;
    }
}

void AppearanceCT::UseAvatarFrame(const int index) {
    CheckAvatarFrame(index);

    const auto iter = avatar_frame_map_.find(index);
    if (iter == avatar_frame_map_.end() || !iter->second.activated)
        return;

    appear_.avatar_frame = index;
}

void AppearanceCT::SyncCache(CacheNode* node) {
    node->avatar = appear_.avatar;
    node->avatarFrame = appear_.avatar_frame;
}

void AppearanceCT::CheckAvatar(const int index) {
    const auto iter = avatar_map_.find(index);
    if (iter == avatar_map_.end())
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
    const auto iter = avatar_frame_map_.find(index);
    if (iter == avatar_frame_map_.end())
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
    request.ParseFromString(dynamic_cast<FPackage *>(pkg)->toString());

    switch (request.operate_type()) {
        case Appearance::SEND_INFO: {
            ct->SendInfo();
        }
        break;
        case Appearance::ACTIVE_AVATAR: {
            ct->ActiveAvatar(request.index(), request.parameter() == 1);
            ct->SendInfo();
        }
        break;
        case Appearance::USE_AVATAR: {
            ct->UseAvatar(request.index());
            ct->SendInfo();
        }
        break;
        case Appearance::ACTIVE_AVATAR_FRAME: {
            ct->ActiveAvatarFrame(request.index(), request.parameter() == 1);
            ct->SendInfo();
        }
        break;
        case Appearance::USE_AVATAR_FRAME: {
            ct->UseAvatarFrame(request.index());
            ct->SendInfo();
        }
        break;
        default:
            break;
    }
}
