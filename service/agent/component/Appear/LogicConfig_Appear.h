#pragma once

#include <Config/LogicConfig.h>
#include <nlohmann/json.hpp>

class ULogicConfig_Appear final : public ILogicConfig {

public:
    int Initial(UConfig *config) override;

private:
    int InitAvatar(const nlohmann::json &data);
};

