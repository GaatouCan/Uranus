#pragma once

#include "../../common.h"


enum class EPluginType {
    INITIALIZE,
    RUNTIME,
    RECEIVE_DATA,
    PLAYER_LOGIN
};

class BASE_API IBasePlugin {

    class UPluginSystem * owner_;

public:
    explicit IBasePlugin(UPluginSystem * owner);
    virtual ~IBasePlugin() = default;

    [[nodiscard]] virtual EPluginType getType() const = 0;
    [[nodiscard]] virtual const char* getPluginName() const = 0;

    [[nodiscard]] UPluginSystem *getOwner() const;
    [[nodiscard]] class  UGameWorld *getWorld() const;
};
