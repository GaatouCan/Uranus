#pragma once

#include "common.h"

class UMonitor;
class UServer;

class BASE_API IPlugin {


public:
    IPlugin() = delete;

    explicit IPlugin(UMonitor* monitor);
    virtual ~IPlugin();

    DISABLE_COPY_MOVE(IPlugin)

    [[nodiscard]] virtual const char *GetPluginName() const = 0;

    [[nodiscard]] UMonitor *GetMonitor() const;
    [[nodiscard]] UServer *GetServer() const;

    virtual void Initial();
    virtual void Start();
    virtual void Stop();

private:
    UMonitor *mMonitor;
};

