#pragma once

#include "../common.h"

#include <vector>
#include <map>
#include <nlohmann/json.hpp>


class BASE_API ILogicConfig {

public:
    ILogicConfig() = default;
    virtual ~ILogicConfig() = default;

    virtual std::vector<std::string> InitialPathList() const;

    virtual int Initial(const std::map<std::string, nlohmann::json> &config) = 0;
};

#define INIT_LOGIC_CONFIG(path, func) \
{ \
    if (const auto iter = config.find(path); iter != config.end()) { \
        if (const auto res = this->func(iter->second); res != 0) { \
            SPDLOG_ERROR("{:<20} - Path[{}] Func[{}] Result[{}]", __FUNCTION__, path, #func, res); \
            return res; \
        } \
    } \
}