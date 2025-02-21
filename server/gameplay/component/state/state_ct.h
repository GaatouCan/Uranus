//
// Created by admin on 25-2-21.
//
#pragma once

#include "../../../player/player_component.h"

class ISerializer;
class Deserializer;

class StateCT final : public IPlayerComponent {

public:
    explicit StateCT(IComponentContext *ctx);
    ~StateCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "State"; }
};
