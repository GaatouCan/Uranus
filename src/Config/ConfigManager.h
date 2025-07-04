#pragma once

#include "LogicConfig.h"

#include <absl/container/flat_hash_map.h>
#include <typeindex>
#include <concepts>

using absl::flat_hash_map;

class BASE_API UConfigManager final {

    flat_hash_map<std::type_index, ILogicConfig *> mConfigMap;

    bool bLoaded;

public:
    UConfigManager();
    ~UConfigManager();

    DISABLE_COPY_MOVE(UConfigManager)

    int LoadConfig(UConfig *config);

    template<class Type>
    requires std::derived_from<Type, ILogicConfig> && (!std::is_same_v<Type, ILogicConfig>)
    void CreateLogicConfig();

    template<class Type>
    requires std::derived_from<Type, ILogicConfig> && (!std::is_same_v<Type, ILogicConfig>)
    Type *GetLogicConfig();
};


template<class Type>
requires std::derived_from<Type, ILogicConfig> && (!std::is_same_v<Type, ILogicConfig>)
inline void UConfigManager::CreateLogicConfig() {
    if (mConfigMap.contains(typeid(Type)))
        return;

    mConfigMap.emplace(typeid(Type), new Type());
}

template<class Type> requires std::derived_from<Type, ILogicConfig> && (!std::is_same_v<Type, ILogicConfig>)
Type *UConfigManager::GetLogicConfig() {
    if (!bLoaded)
        return nullptr;

    const auto it = mConfigMap.find(typeid(Type));
    return it == mConfigMap.end() ? nullptr : it->second;
}
