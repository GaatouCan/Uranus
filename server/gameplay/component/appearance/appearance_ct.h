//
// Created by admin on 25-2-12.
//

#pragma once

#include "../../../player/player_component.h"

#include "appearance.orm.h"


class ITableArray;
class TableResult;

class AppearanceCT final : public IPlayerComponent {

    orm::DBTable_Appearance mAppear;
    std::unordered_map<int32_t, orm::DBTable_Avatar> mAvatarMap;
    std::unordered_map<int32_t, orm::DBTable_AvatarFrame> mAvatarFrameMap;

public:
    explicit AppearanceCT(ComponentModule *module);
    ~AppearanceCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "Appearance"; }

    void Serialize(const std::shared_ptr<Serializer> &s) override;
    void Deserialize(Deserializer &ds) override;

    void GetTableList(std::vector<std::string> &list) const override;

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

