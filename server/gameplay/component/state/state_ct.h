//
// Created by admin on 25-2-21.
//
#pragma once

#include "../../../player/player_component.h"

#include "state.orm.h"


class UStateCT final : public IPlayerComponent {

    orm::DBTable_State state_;

public:
    explicit UStateCT(UComponentModule *module);
    ~UStateCT() override;

    [[nodiscard]] constexpr std::vector<std::string> getTableList() const override {
        return {"state"};
    }

    void serialize(const std::shared_ptr<USerializer> &s) override;
    void deserialize(UDeserializer &ds) override;

    void onLogin() override;

    [[nodiscard]] int32_t getLevel() const;
    [[nodiscard]] int64_t getExp() const;

    void syncCache(FCacheNode* node) override;
};
