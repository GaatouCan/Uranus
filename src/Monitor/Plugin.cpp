#include "Plugin.h"
#include "Monitor.h"

IPluginBase::IPluginBase(UMonitor *monitor)
    : mMonitor(monitor) {
}

IPluginBase::~IPluginBase() {
}

UMonitor *IPluginBase::GetMonitor() const {
    return mMonitor;
}

UServer *IPluginBase::GetServer() const {
    return mMonitor->GetServer();
}

void IPluginBase::Initial() {
}

void IPluginBase::Start() {
}

void IPluginBase::Stop() {
}
