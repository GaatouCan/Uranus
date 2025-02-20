#pragma once

#include "command_object.h"

#include <asio/awaitable.hpp>


class BASE_API IBaseCommand {
protected:
    CommandObject mObject;

public:
    IBaseCommand() = delete;

    explicit IBaseCommand(CommandObject object);
    virtual ~IBaseCommand() = default;

    virtual asio::awaitable<void> Execute() = 0;
};