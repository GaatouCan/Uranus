//
// Created by admin on 25-2-21.
//
#pragma once

#include "../../../player/player_component.h"

#include "state.orm.h"

class ITableArray;
class TableResult;

class StateCT final : public IPlayerComponent {

    orm::DBTable_State mState;

public:
    explicit StateCT(IComponentContext *ctx);
    ~StateCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "State"; }

    void OnLogin() override;

    [[nodiscard]] int32_t GetLevel() const;
    [[nodiscard]] int64_t GetExp() const;

    void SyncCache(CacheNode* node) override;

    void Serialize(Serializer* s) override;
    void Deserialize(Deserializer* ds) override;
};
