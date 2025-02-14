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
    std::vector<orm::DBTable_Avatar> mAvatarList;
    std::vector<orm::DBTable_AvatarFrame> mAvatarFrameList;

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
};

