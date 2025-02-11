#pragma once

#include "../../common.h"


enum class PluginType {
    INITIALIZE,
    RUNTIME,
    RECEIVE_DATA,
    PLAYER_LOGIN
};

class BASE_API IAbstractPlugin {

public:
    virtual ~IAbstractPlugin() = default;

    [[nodiscard]] virtual PluginType GetType() const = 0;
    [[nodiscard]] virtual const char* GetPluginName() const = 0;
};
