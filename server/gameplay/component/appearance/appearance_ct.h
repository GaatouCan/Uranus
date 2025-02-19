//
// Created by admin on 25-2-12.
//

#pragma once

#include "../../../player/player_component.h"

#include "appearance.orm.h"


class ISerializer;
class Deserializer;

class AppearanceCT final : public IPlayerComponent {

    orm::DBTable_Appearance mAppear;
    std::unordered_map<int32_t, orm::DBTable_Avatar> mAvatarMap;
    std::unordered_map<int32_t, orm::DBTable_AvatarFrame> mAvatarFrameMap;

public:
    explicit AppearanceCT(IComponentContext *ctx);
    ~AppearanceCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "Appearance"; }

    ISerializer *Serialize_Appearance(bool &bExpired) const;
    void Deserialize_Appearance(Deserializer &ds);

    ISerializer *Serialize_Avatar(bool &bExpired) const;
    void Deserialize_Avatar(Deserializer &ds);

    ISerializer *Serialize_AvatarFrame(bool &bExpired) const;
    void Deserialize_AvatarFrame(Deserializer &ds);

    void OnLogin() override;

    void SendInfo() const;

    void ActiveAvatar(int index, bool bAutoUse = false);
    void UseAvatar(int index);
};

