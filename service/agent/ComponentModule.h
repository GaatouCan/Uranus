#pragma once

#include "PlayerComponent.h"

#include <absl/container/flat_hash_map.h>
#include <typeindex>
#include <memory>
#include <asio/awaitable.hpp>



class UComponentModule final {

    UPlayer *player_;

    absl::flat_hash_map<std::type_index, std::unique_ptr<IPlayerComponent>> componentMap_;
    std::vector<std::type_index> orderedVector_;

public:
    explicit UComponentModule(UPlayer *player);
    ~UComponentModule();

    [[nodiscard]] UPlayer *GetPlayer() const;

    template<class Type>
    requires std::derived_from<Type, IPlayerComponent>
    Type *CreateComponent() {
        if (componentMap_.contains(typeid(Type)))
            return dynamic_cast<Type *>(componentMap_[typeid(Type)].get());

        auto *component = new Type();

        auto ptr = std::unique_ptr<Type>(component);
        ptr->SetUpModule(this);

        componentMap_.emplace(typeid(Type), std::move(ptr));
        orderedVector_.emplace_back(typeid(Type));

        return component;;
    }

    template<class Type>
    requires std::derived_from<Type, IPlayerComponent>
    Type *GetComponent() const {
        const auto iter = componentMap_.find(typeid(Type));
        return iter == componentMap_.end() ? nullptr : dynamic_cast<Type *>(iter->second.get());
    }

    void OnLogin() const;
    void OnLogout() const;

    void OnDayChange() const;

    void Serialize() const;
    asio::awaitable<void> Deserialize();
};

