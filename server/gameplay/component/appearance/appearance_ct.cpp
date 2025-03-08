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


AppearanceCT::AppearanceCT(UComponentModule *module)
    : IPlayerComponent(module) {

}

AppearanceCT::~AppearanceCT() {
}

void AppearanceCT::serialize(const std::shared_ptr<USerializer> &s) {
    WRITE_TABLE(s, Appearance, appear_)
    WRITE_TABLE_MAP(s, Avatar, avatarMap_)
    WRITE_TABLE_MAP(s, AvatarFrame, avatarFrameMap_)
}

void AppearanceCT::deserialize(UDeserializer &ds) {
    READ_TABLE(&ds, Appearance, appear_)
    READ_TABLE_MAP(&ds, Avatar, avatarMap_)
    READ_TABLE_MAP(&ds, AvatarFrame, avatarFrameMap_)
}

void AppearanceCT::onLogin() {
    if (appear_.pid == 0)
        appear_.pid = getOwner()->getFullID();
}

void AppearanceCT::sendInfo() const {
    Appearance::AppearanceResponse res;

    res.set_current_avatar(appear_.avatar);
    res.set_current_avatar_frame(appear_.avatar_frame);

    for (const auto &val: avatarMap_ | std::views::values) {
        const auto avatar = res.add_avatar();
        avatar->set_index(val.index);
        avatar->set_bactivated(val.activated);
        avatar->set_expired(val.expired_time);
    }

    for (const auto &val: avatarFrameMap_ | std::views::values) {
        const auto avatar_frame = res.add_avatar_frame();
        avatar_frame->set_index(val.index);
        avatar_frame->set_bactivated(val.activated);
        avatar_frame->set_expired(val.expired_time);
    }

    SEND_PACKAGE(this, AppearanceResponse, res);
}

void AppearanceCT::activeAvatar(const int index, const bool bAutoUse) {
    checkAvatar(index);

    auto iter = avatarMap_.find(index);
    if (iter != avatarMap_.end() && iter->second.activated)
        return;

    const auto cfg_op = getWorld()->getConfigManager()->find("appearance.avatar", index);
    if (!cfg_op.has_value())
        return;

    // const auto cfg = cfg_op.value();
    orm::DBTable_Avatar avatar;

    avatar.pid = getOwner()->getFullID();
    avatar.index = index;
    avatar.activated = true;
    avatar.expired_time = utils::UnixTime() + 1000000;

    avatarMap_[index] = avatar;
    iter = avatarMap_.find(index);

    if (bAutoUse) {
        // iter->second.in_used = true;
        appear_.avatar = index;
    }
}

void AppearanceCT::useAvatar(const int index) {
    checkAvatar(index);

    const auto iter = avatarMap_.find(index);
    if (iter == avatarMap_.end() || !iter->second.activated)
        return;

    appear_.avatar = index;
}

void AppearanceCT::activeAvatarFrame(const int index, bool bAutoUse) {
    checkAvatarFrame(index);

    auto iter = avatarFrameMap_.find(index);
    if (iter != avatarFrameMap_.end() && iter->second.activated)
        return;

    const auto cfg_op = getWorld()->getConfigManager()->find("appearance.avatar", index);
    if (!cfg_op.has_value())
        return;

    // const auto cfg = cfg_op.value();
    orm::DBTable_AvatarFrame frame;

    frame.pid = getOwner()->getFullID();
    frame.index = index;
    frame.activated = true;
    frame.expired_time = utils::UnixTime() + 1000000;

    avatarFrameMap_[index] = frame;
    iter = avatarFrameMap_.find(index);

    if (bAutoUse) {
        // iter->second.in_used = true;
        appear_.avatar_frame = index;
    }
}

void AppearanceCT::useAvatarFrame(const int index) {
    checkAvatarFrame(index);

    const auto iter = avatarFrameMap_.find(index);
    if (iter == avatarFrameMap_.end() || !iter->second.activated)
        return;

    appear_.avatar_frame = index;
}

void AppearanceCT::syncCache(FCacheNode* node) {
    node->avatar = appear_.avatar;
    node->avatarFrame = appear_.avatar_frame;
}

void AppearanceCT::checkAvatar(const int index) {
    const auto iter = avatarMap_.find(index);
    if (iter == avatarMap_.end())
        return;

    const auto elem = &iter->second;
    if (!elem->activated)
        return;

    if (elem->expired_time < 0)
        return;

    if (elem->expired_time >= 0 && elem->expired_time < utils::UnixTime())
        elem->activated = false;
}

void AppearanceCT::checkAvatarFrame(const int index) {
    const auto iter = avatarFrameMap_.find(index);
    if (iter == avatarFrameMap_.end())
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

    const auto ct = std::dynamic_pointer_cast<UPlayer>(plr)->getComponent<AppearanceCT>();
    if (ct == nullptr)
        return;

    Appearance::AppearanceRequest request;
    request.ParseFromString(dynamic_cast<FPackage *>(pkg)->toString());

    switch (request.operate_type()) {
        case Appearance::SEND_INFO: {
            ct->sendInfo();
        }
        break;
        case Appearance::ACTIVE_AVATAR: {
            ct->activeAvatar(request.index(), request.parameter() == 1);
            ct->sendInfo();
        }
        break;
        case Appearance::USE_AVATAR: {
            ct->useAvatar(request.index());
            ct->sendInfo();
        }
        break;
        case Appearance::ACTIVE_AVATAR_FRAME: {
            ct->activeAvatarFrame(request.index(), request.parameter() == 1);
            ct->sendInfo();
        }
        break;
        case Appearance::USE_AVATAR_FRAME: {
            ct->useAvatarFrame(request.index());
            ct->sendInfo();
        }
        break;
        default:
            break;
    }
}
