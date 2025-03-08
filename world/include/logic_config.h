#pragma once

#include <utility>
#include <vector>
#include <concepts>
#include <nlohmann/json.hpp>

#include "common.h"

class BASE_API ILogicConfig {
public:
    ILogicConfig() = delete;

    explicit ILogicConfig(std::vector<nlohmann::json> config_list)
        : list_(std::move(config_list)) {}

    virtual ~ILogicConfig() = default;

    DISABLE_COPY_MOVE(ILogicConfig)

    virtual void onReload(std::vector<nlohmann::json> config_list) {
        list_ = std::move(config_list);
    }

    virtual void init() {}

protected:
    std::vector<nlohmann::json> list_;
};

template<typename T>
concept CLogicConfigType = std::derived_from<T, ILogicConfig>;
