#include "../../../include/system/plugin/base_plugin.h"
#include "../../../include/system/plugin/plugin_system.h"


IBasePlugin::IBasePlugin(UPluginSystem *owner)
    : owner_(owner) {
}

UPluginSystem * IBasePlugin::getOwner() const {
    return owner_;
}

UGameWorld * IBasePlugin::getWorld() const {
    return owner_->getWorld();
}
