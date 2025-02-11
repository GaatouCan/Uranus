#pragma once

#include "../../common.h"


enum class PluginType {
    INITIALIZE,
    RUNTIME,
    RECEIVE_DATA,
    PLAYER_LOGIN
};

class BASE_API IBasePlugin {

public:
    virtual ~IBasePlugin() = default;

    [[nodiscard]] virtual PluginType GetType() const = 0;
    [[nodiscard]] virtual const char* GetPluginName() const = 0;
};
