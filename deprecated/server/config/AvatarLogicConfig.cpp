#include "AvatarLogicConfig.h"

#include <spdlog/spdlog.h>


UAvatarLogicConfig::UAvatarLogicConfig(std::vector<nlohmann::json> configList)
    : ILogicConfig(std::move(configList)) {

}

UAvatarLogicConfig::~UAvatarLogicConfig() {
}
