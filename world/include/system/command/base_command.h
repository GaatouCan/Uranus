#pragma once

#include "command_object.h"

#include <asio/awaitable.hpp>


class BASE_API IBaseCommand {

    class UCommandSystem *owner_;

protected:
    UCommandObject object_;

public:
    IBaseCommand() = delete;

    IBaseCommand(UCommandSystem *owner, UCommandObject object);
    virtual ~IBaseCommand() = default;

    virtual asio::awaitable<void> Execute() = 0;

    [[nodiscard]] UCommandSystem *GetOwner() const;
    [[nodiscard]] class UGameWorld *GetWorld() const;
};