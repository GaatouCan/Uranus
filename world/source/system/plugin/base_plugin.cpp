#include "../../../include/system/plugin/base_plugin.h"
#include "../../../include/system/plugin/plugin_system.h"


IBasePlugin::IBasePlugin(PluginSystem *owner)
    : owner_(owner) {
}

PluginSystem * IBasePlugin::GetOwner() const {
    return owner_;
}

GameWorld * IBasePlugin::GetWorld() const {
    return owner_->GetWorld();
}
