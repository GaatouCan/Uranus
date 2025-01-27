#pragma once

#include "../../common.h"


enum class EPluginType {
    INITIALIZE,
    RUNTIME,
    RECEIVE_DATA,
    PLAYER_LOGIN
};

class BASE_API IAbstractPlugin {

public:
    virtual ~IAbstractPlugin() = default;

    virtual EPluginType GetType() const = 0;
    virtual const char* GetPluginName() const = 0;
};
