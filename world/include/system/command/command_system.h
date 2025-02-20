#pragma once

#include "../../sub_system.h"

class BASE_API CommandSystem final : public ISubSystem {

public:
    explicit CommandSystem(GameWorld *world);
    ~CommandSystem() override;

    GET_SYSTEM_NAME(CommandSystem)

    void Init() override;
};
