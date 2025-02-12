#pragma once

#include "../../sub_system.h"

class BASE_API DatabaseSystem final : public ISubSystem {

public:
    explicit DatabaseSystem(GameWorld *world);
    ~DatabaseSystem() override;

    GET_SYSTEM_NAME(DatabaseSystem)

    void Init() override;
};