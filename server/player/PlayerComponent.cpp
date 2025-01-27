#include "PlayerComponent.h"
#include "ComponentModule.h"
#include "Player.h"


IPlayerComponent::IPlayerComponent(IComponentContext *ctx)
    : mContext(ctx) {
}

IPlayerComponent::~IPlayerComponent() {
}

IComponentContext * IPlayerComponent::GetComponentContext() const {
    return mContext;
}

UComponentModule * IPlayerComponent::GetModule() const {
    return mContext->GetModule();
}

UPlayer * IPlayerComponent::GetOwner() const {
    return mContext->GetModule()->GetOwner();
}

void IPlayerComponent::OnLogin() {
}

void IPlayerComponent::OnLogout() {
}

void IPlayerComponent::OnDayChange(bool bLogin) {
}

void IPlayerComponent::SyncCache(FCacheNode *node) {
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
