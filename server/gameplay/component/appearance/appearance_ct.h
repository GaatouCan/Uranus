//
// Created by admin on 25-2-12.
//

#pragma once

#include "../../../player/player_component.h"

#include "appearance.orm.h"


class ITableVector;
class Deserializer;

class AppearanceCT final : public IPlayerComponent {

    orm::DBTable_Appearance mAppear;
    std::unordered_map<int32_t, orm::DBTable_Avatar> mAvatarMap;
    std::unordered_map<int32_t, orm::DBTable_AvatarFrame> mAvatarFrameMap;

public:
    explicit AppearanceCT(IComponentContext *ctx);
    ~AppearanceCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "Appearance"; }

    ITableVector *Serialize_Appearance(bool &bExpired) const;
    void Deserialize_Appearance(Deserializer &ds);

    ITableVector *Serialize_Avatar(bool &bExpired) const;
    void Deserialize_Avatar(Deserializer &ds);

    ITableVector *Serialize_AvatarFrame(bool &bExpired) const;
    void Deserialize_AvatarFrame(Deserializer &ds);

    void OnLogin() override;

    void SendInfo() const;

    void ActiveAvatar(int index, bool bAutoUse = false);
    void UseAvatar(int index);

    void ActiveAvatarFrame(int index, bool bAutoUse = false);
    void UseAvatarFrame(int index);

    void SyncCache(struct CacheNode* node) override;

private:
    void CheckAvatar(int index);
    void CheckAvatarFrame(int index);
};

