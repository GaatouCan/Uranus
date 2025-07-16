#pragma once

#include "Common.h"

class UMonitor;
class UServer;

class BASE_API IPluginBase {


public:
    IPluginBase() = delete;

    explicit IPluginBase(UMonitor* monitor);
    virtual ~IPluginBase();

    DISABLE_COPY_MOVE(IPluginBase)

    [[nodiscard]] virtual const char *GetPluginName() const = 0;

    [[nodiscard]] UMonitor *GetMonitor() const;
    [[nodiscard]] UServer *GetServer() const;

    virtual void Initial();
    virtual void Start();
    virtual void Stop();

private:
    UMonitor *mMonitor;
};

