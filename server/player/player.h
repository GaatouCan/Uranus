#pragma once

#include "component_module.h"
#include "event_module.h"

#include "scene/base_player.h"


struct EP_PlayerLogin final : IEventParam {
    int64_t pid;
};

struct EP_PlayerLogout final : IEventParam {
    int64_t pid;
};

class UPlayer final : public IBasePlayer {

    ATimePoint loginTime_;
    ATimePoint logoutTime_;

    ComponentModule componentModule_;
    UEventModule eventModule_;

public:
    UPlayer() = delete;

    explicit UPlayer(const AConnectionPointer &conn);
    ~UPlayer() override;

    ComponentModule &getComponentModule() noexcept { return componentModule_; }
    UEventModule &getEventModule() noexcept { return eventModule_; }

    void onDayChange();

    awaitable<void> onLogin();
    void onLogout(bool is_force = false, const std::string &other_address = "");

    bool isOnline() const;

    void send(uint32_t id, std::string_view data) const;
    void send(uint32_t id, const std::stringstream &ss) const;

    void syncCache(CacheNode *node);

    void dispatchEvent(EEvent event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *getComponent() {
        return componentModule_.getComponent<T>();
    }
};

#define SEND_PACKAGE(sender, proto, data) \
    (sender)->send(static_cast<uint32_t>(protocol::EProtoType::proto), (data).SerializeAsString());

#define BUILD_PACKAGE(pkg, proto, data) \
    (pkg)->setPackageID(static_cast<uint32_t>(protocol::EProtoType::proto)).setData((data).SerializeAsString());
