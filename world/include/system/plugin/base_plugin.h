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

    [[nodiscard]] virtual EPluginType GetType() const = 0;
    [[nodiscard]] virtual const char* GetPluginName() const = 0;

    [[nodiscard]] UPluginSystem * GetOwner() const;
    [[nodiscard]] class  UGameWorld *GetWorld() const;
};
