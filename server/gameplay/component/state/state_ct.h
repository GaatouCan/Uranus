//
// Created by admin on 25-2-21.
//
#pragma once

#include "../../../player/player_component.h"

#include "state.orm.h"


class StateCT final : public IPlayerComponent {

    orm::DBTable_State state_;

public:
    explicit StateCT(ComponentModule *module);
    ~StateCT() override;

    [[nodiscard]] constexpr std::vector<std::string> getTableList() const override {
        return {"state"};
    }

    void serialize(const std::shared_ptr<USerializer> &s) override;
    void deserialize(UDeserializer &ds) override;

    void onLogin() override;

    [[nodiscard]] int32_t GetLevel() const;
    [[nodiscard]] int64_t GetExp() const;

    void syncCache(CacheNode* node) override;
};
