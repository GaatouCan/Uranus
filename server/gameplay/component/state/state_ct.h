//
// Created by admin on 25-2-21.
//
#pragma once

#include "../../../player/player_component.h"

#include "state.orm.h"


class StateCT final : public IPlayerComponent {

    orm::DBTable_State mState;

public:
    explicit StateCT(ComponentModule *module);
    ~StateCT() override;

    [[nodiscard]] constexpr std::vector<std::string> GetTableList() const override {
        return {"state"};
    }

    void Serialize(const std::shared_ptr<Serializer> &s) override;
    void Deserialize(Deserializer &ds) override;

    void OnLogin() override;

    [[nodiscard]] int32_t GetLevel() const;
    [[nodiscard]] int64_t GetExp() const;

    void SyncCache(CacheNode* node) override;
};
