#include "Plugin.h"
#include "Monitor.h"

IPlugin::IPlugin(UMonitor *monitor)
    : mMonitor(monitor) {
}

IPlugin::~IPlugin() {
}

UMonitor *IPlugin::GetMonitor() const {
    return mMonitor;
}

UServer *IPlugin::GetServer() const {
    return mMonitor->GetServer();
}

void IPlugin::Initial() {
}

void IPlugin::Start() {
}

void IPlugin::Stop() {
}
