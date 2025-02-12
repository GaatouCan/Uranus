//
// Created by admin on 25-2-12.
//

#pragma once

#include "../../../player/player_component.h"


class AppearanceCT final : public IPlayerComponent {

public:
    explicit AppearanceCT(IComponentContext *ctx);
    ~AppearanceCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "Appearance"; }
};

