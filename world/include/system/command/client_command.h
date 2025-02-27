#pragma once

#include "base_command.h"

class BASE_API IClientCommand : public IBaseCommand {

    int32_t sender_;

public:
    IClientCommand(CommandSystem *owner, CommandObject object);

    void SetSender(int32_t sender);
    [[nodiscard]] int32_t GetSender() const;
};