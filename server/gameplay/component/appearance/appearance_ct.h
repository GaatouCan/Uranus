//
// Created by admin on 25-2-12.
//

#pragma once

#include "../../../player/player_component.h"

#include "appearance.orm.h"


class AppearanceCT final : public IPlayerComponent {

    orm::DBTable_Appearance appear_;
    std::unordered_map<int32_t, orm::DBTable_Avatar> avatarMap_;
    std::unordered_map<int32_t, orm::DBTable_AvatarFrame> avatarFrameMap_;

public:
    explicit AppearanceCT(UComponentModule *module);
    ~AppearanceCT() override;

    [[nodiscard]] constexpr std::vector<std::string> getTableList() const  override {
        return {"appearance", "avatar", "avatar_frame"};
    }

    void serialize(const std::shared_ptr<USerializer> &s) override;
    void deserialize(UDeserializer &ds) override;

    void onLogin() override;

    void sendInfo() const;

    void activeAvatar(int index, bool bAutoUse = false);
    void useAvatar(int index);

    void activeAvatarFrame(int index, bool bAutoUse = false);
    void useAvatarFrame(int index);

    void syncCache(FCacheNode* node) override;

private:
    void checkAvatar(int index);
    void checkAvatarFrame(int index);
};

