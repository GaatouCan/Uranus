#pragma once

#include "component_module.h"
#include "event_module.h"

#include "scene/base_player.h"
#include "platform_info.h"


struct EP_PlayerLogin final : IEventParam {
    int64_t pid;
};

struct EP_PlayerLogout final : IEventParam {
    int64_t pid;
};

class Player final : public IBasePlayer {

    TimePoint mLoginTime;
    TimePoint mLogoutTime;

    ComponentModule mComponentModule;
    EventModule mEventModule;

    PlatformInfo mPlatform;

public:
    Player() = delete;

    explicit Player(ConnectionPointer conn);
    ~Player() override;

    ComponentModule &GetComponentModule() noexcept { return mComponentModule; }
    EventModule &GetEventModule() noexcept { return mEventModule; }

    void OnDayChange();

    awaitable<void> OnLogin();
    void OnLogout(bool is_force = false, const std::string &other_address = "");

    bool IsOnline() const;

    void Send(uint32_t id, std::string_view data) const;
    void Send(uint32_t id, const std::stringstream &ss) const;

    void SyncCache(CacheNode *node);

    void DispatchEvent(Event event, IEventParam *param, DispatchType type = DispatchType::PUSH_QUEUE);

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *GetComponent() {
        return mComponentModule.GetComponent<T>();
    }
};

#define SEND_PACKAGE(sender, proto, data) \
    (sender)->Send(static_cast<uint32_t>(protocol::ProtoType::proto), (data).SerializeAsString());

#define BUILD_PACKAGE(pkg, proto, data) \
    (pkg)->SetPackageID(static_cast<uint32_t>(protocol::ProtoType::proto)).SetData((data).SerializeAsString());
