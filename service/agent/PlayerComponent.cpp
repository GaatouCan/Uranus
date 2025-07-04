#include "PlayerComponent.h"
#include "ComponentModule.h"
#include "Player.h"


IPlayerComponent::IPlayerComponent()
    : mModule(nullptr) {
}

IPlayerComponent::~IPlayerComponent() {
}

void IPlayerComponent::SetUpModule(UComponentModule *module) {
    mModule = module;
}

UComponentModule *IPlayerComponent::GetModule() const {
    return mModule;
}

UPlayer *IPlayerComponent::GetPlayer() const {
    if (mModule == nullptr)
        return nullptr;
    return mModule->GetPlayer();
}

int64_t IPlayerComponent::GetPlayerID() const {
    if (mModule == nullptr)
        return 0;
    return mModule->GetPlayer()->GetPlayerID();
}

void IPlayerComponent::OnLogin() {
}

void IPlayerComponent::OnLogout() {
}

void IPlayerComponent::OnDayChange() {
}

void IPlayerComponent::Serialize(const std::shared_ptr<FSerializer> &s) {
}

void IPlayerComponent::Deserialize(FDeserializer &ds) {
}

void IPlayerComponent::SendToClient(const uint32_t id, const std::string &data) const {
    if (const auto *plr = GetPlayer()) {
        plr->SendToClient(id, data);
    }
}

void IPlayerComponent::SendToService(const int32_t sid, const uint32_t id, const std::string &data) const {
    if (const auto *plr = GetPlayer()) {
        plr->SendToService(sid, id, data);
    }
}

void IPlayerComponent::SendToPlayer(const int64_t pid, const uint32_t id, const std::string &data) const {
    if (const auto *plr = GetPlayer()) {
        plr->SendToPlayer(pid, id, data);
    }
}
