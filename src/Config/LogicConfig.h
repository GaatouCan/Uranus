#pragma once

#include "../common.h"

class UConfig;

class BASE_API ILogicConfig {

public:
    ILogicConfig() = default;
    virtual ~ILogicConfig() = default;

    virtual int Initial(UConfig *config) = 0;
};

#define INIT_LOGIC_CONFIG(path, func) \
{ \
    if (const auto op = config->Find(path); op.has_value()) { \
        if (const auto res = this->func(op.value()); res != 0) { \
            SPDLOG_ERROR("{:<20} - Path[{}] Func[{}] Result[{}]", __FUNCTION__, path, #func, res); \
            return res; \
        } \
    } \
}