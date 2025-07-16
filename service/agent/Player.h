#pragma once

#include <Gateway/PlayerAgent.h>
#include <Config/ConfigManager.h>

#include "ComponentModule.h"
#include "ProtoRoute.h"


class UPlayer final : public IPlayerAgent {

    DECLARE_SERVICE(UPlayer, IPlayerAgent)

    UProtoRoute mRoute;
    UComponentModule mComponent;
    UConfigManager mConfig;

public:
    UPlayer();
    ~UPlayer() override;

    bool Initial(const std::shared_ptr<IPackageBase> &pkg) override;
    bool Start() override;
    void Stop() override;

    void OnPackage(const std::shared_ptr<IPackageBase> &pkg) override;

    void SendToClient(uint32_t id, const std::string &data) const;

    void SendToService(int32_t sid, uint32_t id, const std::string &data) const;
    void SendToService(const std::string &name, uint32_t id, const std::string &data) const;

    void SendToPlayer(int64_t pid, uint32_t id, const std::string &data) const;

    UComponentModule &GetComponentModule();

    template<class Type>
    requires std::derived_from<Type, IPlayerComponent>
    Type *GetComponent() const {
        return mComponent.GetComponent<Type>();
    }
};

