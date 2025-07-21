#pragma once

#include <Service/Service.h>
#include <Config/ConfigManager.h>

#include <memory>
#include <typeindex>
#include <absl/container/flat_hash_map.h>

#include "ProtoRoute.h"
#include "Manager.h"


class UGameWorld final : public IServiceBase {

    DECLARE_SERVICE(UGameWorld, IServiceBase)

public:
    UGameWorld();
    ~UGameWorld() override;

    [[nodiscard]] std::string GetServiceName() const override {
        return "Game World";
    }

    bool Initial(const std::shared_ptr<IPackageInterface> &pkg) override;
    bool Start() override;
    void Stop() override;

    template<CManagerType Type, class ... Args>
    Type *CreateManager(Args &&... args) {
        if (state_ != EServiceState::CREATED)
            return nullptr;

        if (mManagerMap.contains(typeid(Type))) {
            return dynamic_cast<Type *>(mManagerMap[typeid(Type)].get());
        }

        auto *manager = new Type(this, std::forward<Args>(args)...);
        auto ptr = std::unique_ptr<Type>(manager);

        mManagerMap.emplace(typeid(Type), std::move(ptr));
        mOrdered.emplace_back(typeid(Type));

        return manager;
    }

    template<CManagerType Type>
    Type *GetManager() {
        if (state_ == EServiceState::TERMINATED)
            return nullptr;

        const auto iter = mManagerMap.find(typeid(Type));
        return iter == mManagerMap.end() ? nullptr : dynamic_cast<Type *>(iter->second.get());
    }

    void OnPackage(const std::shared_ptr<IPackageInterface> &pkg) override;

private:
    UProtoRoute mRoute;
    UConfigManager mConfig;

    absl::flat_hash_map<std::type_index, std::unique_ptr<IManager>> mManagerMap;
    std::vector<std::type_index> mOrdered;
};

