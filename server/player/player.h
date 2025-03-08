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

class Player final : public IBasePlayer {

    ATimePoint login_time_;
    ATimePoint logout_time_;

    ComponentModule component_module_;
    EventModule event_module_;

public:
    Player() = delete;

    explicit Player(const AConnectionPointer &conn);
    ~Player() override;

    ComponentModule &GetComponentModule() noexcept { return component_module_; }
    EventModule &GetEventModule() noexcept { return event_module_; }

    void OnDayChange();

    awaitable<void> OnLogin();
    void OnLogout(bool is_force = false, const std::string &other_address = "");

    bool IsOnline() const;

    void Send(uint32_t id, std::string_view data) const;
    void Send(uint32_t id, const std::stringstream &ss) const;

    void SyncCache(CacheNode *node);

    void DispatchEvent(EEvent event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *GetComponent() {
        return component_module_.GetComponent<T>();
    }
};

#define SEND_PACKAGE(sender, proto, data) \
    (sender)->Send(static_cast<uint32_t>(protocol::ProtoType::proto), (data).SerializeAsString());

#define BUILD_PACKAGE(pkg, proto, data) \
    (pkg)->SetPackageID(static_cast<uint32_t>(protocol::ProtoType::proto)).SetData((data).SerializeAsString());
