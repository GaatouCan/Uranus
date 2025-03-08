#pragma once

#include "base_command.h"

class BASE_API IClientCommand : public IBaseCommand {

    int32_t sender_;

public:
    IClientCommand(UCommandSystem *owner, UCommandObject object);

    void setSender(int32_t sender);
    [[nodiscard]] int32_t setSender() const;
};