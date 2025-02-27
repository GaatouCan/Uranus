#pragma once

#include "command_object.h"

#include <asio/awaitable.hpp>


class BASE_API IBaseCommand {

    class CommandSystem *owner_;

protected:
    CommandObject object_;

public:
    IBaseCommand() = delete;

    IBaseCommand(CommandSystem *owner, CommandObject object);
    virtual ~IBaseCommand() = default;

    virtual asio::awaitable<void> Execute() = 0;

    [[nodiscard]] CommandSystem *GetOwner() const;
    [[nodiscard]] class GameWorld *GetWorld() const;
};