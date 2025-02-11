#pragma once

#include "ComponentModule.h"
#include "EventModule.h"

#include "scene/BasePlayer.h"
#include "PlatformInfo.h"


struct EP_PlayerLogin final : IEventParam {
    int64_t pid;
};

struct EP_PlayerLogout final : IEventParam {
    int64_t pid;
};

class Player final : public IBasePlayer {

    TimePoint login_time_;
    TimePoint logout_time_;

    ComponentModule component_module_;
    EventModule event_module_;

    PlatformInfo platform_;

public:
    Player() = delete;

    explicit Player(ConnectionPointer conn);
    ~Player() override;

    ComponentModule &GetComponentModule();
    EventModule &GetEventModule();

    void OnDayChange();

    awaitable<void> OnLogin();
    void OnLogout(bool is_force = false, const std::string &other_address = "");

    bool IsOnline() const;

    void Send(uint32_t id, std::string_view data) const;
    void Send(uint32_t id, const std::stringstream &ss) const;

    void SyncCache(CacheNode *node);

    void DispatchEvent(Event event, IEventParam *param, DispatchType type = DispatchType::PUSH_QUEUE);
};

#define SEND_PACKAGE(sender, proto, data) \
    (sender)->Send(static_cast<int32_t>(protocol::ProtoType::proto), (data).SerializeAsString());

#define BUILD_PACKAGE(pkg, proto, data) \
    (pkg)->SetPackageID(static_cast<int32_t>(protocol::ProtoType::proto)).SetData((data).SerializeAsString());
