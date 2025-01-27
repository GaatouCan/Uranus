#pragma once

#include "CommandObject.h"

#include <asio.hpp>


class IBaseCommand {

protected:
    UCommandObject mParam;

public:
    IBaseCommand() = delete;

    explicit IBaseCommand(UCommandObject param);
    virtual ~IBaseCommand() = default;

    virtual asio::awaitable<bool> Execute() = 0;
};

