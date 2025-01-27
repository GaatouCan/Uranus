#pragma once

#include "ComponentModule.h"
#include "EventModule.h"

#include "scene/BasePlayer.h"
#include "PlatformInfo.h"


struct FEP_PlayerLogin final : IEventParam {
    uint64_t pid;
};

struct FEP_PlayerLogout final : IEventParam {
    uint64_t pid;
};

class UPlayer final : public IBasePlayer {

    ATimePoint mLoginTime;
    ATimePoint mLogoutTime;

    UComponentModule mComponentModule;
    UEventModule mEventModule;

    FPlatformInfo mPlatform;

public:
    UPlayer() = delete;

    explicit UPlayer(AConnectionPointer conn);
    ~UPlayer() override;

    UComponentModule &GetComponentModule();
    UEventModule &GetEventModule();

    void OnDayChange();

    awaitable<void> OnLogin();
    void OnLogout(bool bForce = false, const std::string &otherAddress = "");

    void OnEnterScene(IAbstractScene *scene) override;
    void OnLeaveScene(IAbstractScene *scene) override;

    bool IsOnline() const;

    void Send(int32_t id, std::string_view data) const;
    void Send(int32_t id, const std::stringstream &ss) const;

    void SyncCache(FCacheNode *node);

    void DispatchEvent(EEvent event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);
};

#define SEND_PACKAGE(sender, proto, data) \
    (sender)->Send(static_cast<int32_t>(protocol::EProtoType::proto), (data).SerializeAsString());

#define BUILD_PACKAGE(pkg, proto, data) \
    (pkg)->SetPackageID(static_cast<int32_t>(protocol::EProtoType::proto)).SetData((data).SerializeAsString());
