#include "PlayerComponent.h"
#include "ComponentModule.h"
#include "Player.h"


IPlayerComponent::IPlayerComponent(IComponentContext *ctx)
    : ctx_(ctx) {
}

IPlayerComponent::~IPlayerComponent() {
}

IComponentContext * IPlayerComponent::GetComponentContext() const {
    return ctx_;
}

ComponentModule * IPlayerComponent::GetModule() const {
    return ctx_->GetModule();
}

Player * IPlayerComponent::GetOwner() const {
    return ctx_->GetModule()->GetOwner();
}

GameWorld * IPlayerComponent::GetWorld() const {
    return GetOwner()->GetWorld();
}

void IPlayerComponent::OnLogin() {
}

void IPlayerComponent::OnLogout() {
}

void IPlayerComponent::OnDayChange(bool is_login) {
}

void IPlayerComponent::SyncCache(CacheNode *node) {
}

void IPlayerComponent::Send(IPackage *pkg) const {
    if (const auto plr = GetOwner(); plr != nullptr) {
        plr->SendPackage(pkg);
    }
}

void IPlayerComponent::Send(const int32_t id, const std::string_view data) const {
    if (const auto plr = GetOwner(); plr != nullptr) {
        plr->Send(id, data);
    }
}

void IPlayerComponent::Send(const int32_t id, const std::stringstream &ss) const {
    if (const auto plr = GetOwner(); plr != nullptr) {
        plr->Send(id, ss);
    }
}
