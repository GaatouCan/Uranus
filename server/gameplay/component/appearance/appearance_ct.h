//
// Created by admin on 25-2-12.
//

#pragma once

#include "../../../player/player_component.h"

#include "appearance.orm.h"


class ISerializer;
class Deserializer;

class AppearanceCT final : public IPlayerComponent {

    orm::DBTable_Appearance mAppear;

public:
    explicit AppearanceCT(IComponentContext *ctx);
    ~AppearanceCT() override;

    [[nodiscard]] constexpr const char * GetComponentName() const override { return "Appearance"; }

    ISerializer *Serialize_Appearance(bool &bExpired) const;
    void Deserialize_Appearance(Deserializer &ds);
};

