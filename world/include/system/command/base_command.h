#pragma once

#include "command_object.h"

#include <asio/awaitable.hpp>


class BASE_API IBaseCommand {

    class CommandSystem *mOwner;

protected:
    CommandObject mObject;

public:
    IBaseCommand() = delete;

    IBaseCommand(CommandSystem *owner, CommandObject object);
    virtual ~IBaseCommand() = default;

    virtual asio::awaitable<void> Execute() = 0;

    [[nodiscard]] CommandSystem *GetOwner() const;
    [[nodiscard]] class GameWorld *GetWorld() const;
};