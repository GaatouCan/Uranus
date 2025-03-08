//
// Created by admin on 25-2-12.
//

#pragma once

#include "../../../player/player_component.h"

#include "appearance.orm.h"


class AppearanceCT final : public IPlayerComponent {

    orm::DBTable_Appearance appear_;
    std::unordered_map<int32_t, orm::DBTable_Avatar> avatar_map_;
    std::unordered_map<int32_t, orm::DBTable_AvatarFrame> avatar_frame_map_;

public:
    explicit AppearanceCT(ComponentModule *module);
    ~AppearanceCT() override;

    [[nodiscard]] constexpr std::vector<std::string> getTableList() const  override {
        return {"appearance", "avatar", "avatar_frame"};
    }

    void serialize(const std::shared_ptr<USerializer> &s) override;
    void deserialize(UDeserializer &ds) override;

    void onLogin() override;

    void SendInfo() const;

    void ActiveAvatar(int index, bool bAutoUse = false);
    void UseAvatar(int index);

    void ActiveAvatarFrame(int index, bool bAutoUse = false);
    void UseAvatarFrame(int index);

    void syncCache(struct CacheNode* node) override;

private:
    void CheckAvatar(int index);
    void CheckAvatarFrame(int index);
};

