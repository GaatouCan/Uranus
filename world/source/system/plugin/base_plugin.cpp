#include "../../../include/system/plugin/base_plugin.h"
#include "../../../include/system/plugin/plugin_system.h"


IBasePlugin::IBasePlugin(UPluginSystem *owner)
    : owner_(owner) {
}

UPluginSystem * IBasePlugin::GetOwner() const {
    return owner_;
}

UGameWorld * IBasePlugin::GetWorld() const {
    return owner_->GetWorld();
}
