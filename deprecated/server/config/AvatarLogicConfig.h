#pragma once

#include <system/config/LogicConfig.h>


class UAvatarLogicConfig final : public ILogicConfig {

public:
    explicit UAvatarLogicConfig(std::vector<nlohmann::json> configList);
    ~UAvatarLogicConfig() override;
};

