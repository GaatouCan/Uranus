#pragma once

#include "../../../player/PlayerComponent.h"

#include <system/database/Serializer.h>
#include <system/database/Deserializer.h>
#include <orm/appearance.orm.h>

class UAppearanceCT final : public IPlayerComponent {

    orm::UDBTable_Appearance mAppearance;
    std::map<uint32_t, orm::UDBTable_Avatar> mAvatarMap;
    std::map<uint32_t, orm::UDBTable_AvatarFrame> mAvatarFrameMap;

public:
    explicit UAppearanceCT(IComponentContext *ctx);

    [[nodiscard]] constexpr const char * GetComponentName() const override {
        return "UAppearanceCT";
    }

    ISerializer *Serialize_Appearance(bool &bExpired) const;
    void Deserialize_Appearance(UDeserializer &ds);

    ISerializer *Serialize_Avatar(bool &bExpired) const;
    void Deserialize_Avatar(UDeserializer &ds);

    ISerializer *Serialize_AvatarFrame(bool &bExpired) const;
    void Deserialize_AvatarFrame(UDeserializer &ds);

    void SendInfo() const;

    void ActiveAvatar(int index);
};

