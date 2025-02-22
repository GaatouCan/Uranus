//
// Created by admin on 25-2-21.
//
#pragma once

#include "../../../player/player_component.h"

#include "state.orm.h"

class ISerializer;
class Deserializer;

class StateCT final : public IPlayerComponent {

    orm::DBTable_State mState;

public:
    explicit StateCT(IComponentContext *ctx);
    ~StateCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "State"; }

    ISerializer *Serialize_State(bool &bExpired) const;
    void Deserialize_State(Deserializer &ds);

    [[nodiscard]] int32_t GetLevel() const;
    [[nodiscard]] int64_t GetExp() const;
};
