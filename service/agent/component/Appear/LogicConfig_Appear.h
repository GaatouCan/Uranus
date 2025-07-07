#pragma once

#include <Config/LogicConfig.h>
#include <nlohmann/json.hpp>

class ULogicConfig_Appear final : public ILogicConfig {

public:
    constexpr std::vector<std::string> InitialPathList() const override {
        return {"appear.avatar"};
    }

    int Initial(const std::map<std::string, nlohmann::json> &config) override;

private:
    int InitAvatar(const nlohmann::json &data);
};

