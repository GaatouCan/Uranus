#pragma once

#include <service/service.h>

#include <memory>
#include <typeindex>
#include <absl/container/flat_hash_map.h>

#include "proto_route.h"
#include "manager.h"


class UGameWorld final : public IService {

    DECLARE_SERVICE(UGameWorld, IService)

public:
    explicit UGameWorld(UContext *context);
    ~UGameWorld() override;

    [[nodiscard]] std::string GetServiceName() const override {
        return "Game World";
    }

    void Initial(const std::shared_ptr<IPackage> &pkg) override;
    void Start() override;
    void Stop() override;

    template<CManagerType Type, class ... Args>
    Type *CreateManager(Args &&... args) {
        if (mState != EServiceState::CREATED)
            return nullptr;

        if (managerMap_.contains(typeid(Type))) {
            return dynamic_cast<Type *>(managerMap_[typeid(Type)].get());
        }

        auto *manager = new Type(this, std::forward<Args>(args)...);
        auto ptr = std::unique_ptr<Type>(manager);

        managerMap_.emplace(typeid(Type), std::move(ptr));
        ordered_.emplace_back(typeid(Type));

        return manager;
    }

    template<CManagerType Type>
    Type *GetManager() {
        if (mState == EServiceState::TERMINATED)
            return nullptr;

        const auto iter = managerMap_.find(typeid(Type));
        return iter == managerMap_.end() ? nullptr : dynamic_cast<Type *>(iter->second.get());
    }

    void OnPackage(const std::shared_ptr<IPackage> &pkg) override;

private:
    UProtoRoute route_;

    absl::flat_hash_map<std::type_index, std::unique_ptr<IManager>> managerMap_;
    std::vector<std::type_index> ordered_;
};

