#include "player_component.h"
#include "component_module.h"
#include "player.h"


IPlayerComponent::IPlayerComponent(UComponentModule *module)
    : module_(module) {
}

IPlayerComponent::~IPlayerComponent() {
}

UComponentModule * IPlayerComponent::getModule() const {
    return module_;
}

UPlayer * IPlayerComponent::getOwner() const {
    return module_->getOwner();
}

UGameWorld * IPlayerComponent::getWorld() const {
    return getOwner()->getWorld();
}

void IPlayerComponent::onLogin() {
}

void IPlayerComponent::onLogout() {
}

void IPlayerComponent::onDayChange(bool is_login) {
}

void IPlayerComponent::syncCache(FCacheNode *node) {
}

void IPlayerComponent::send(IPackage *pkg) const {
    if (const auto plr = getOwner(); plr != nullptr) {
        plr->sendPackage(pkg);
    }
}

void IPlayerComponent::send(const int32_t id, const std::string_view data) const {
    if (const auto plr = getOwner(); plr != nullptr) {
        plr->send(id, data);
    }
}

void IPlayerComponent::send(const int32_t id, const std::stringstream &ss) const {
    if (const auto plr = getOwner(); plr != nullptr) {
        plr->send(id, ss);
    }
}
