#include "LogicConfig_Appear.h"

#include <Config/Config.h>
#include <spdlog/spdlog.h>


int ULogicConfig_Appear::Initial(const std::map<std::string, nlohmann::json> &config) {
    INIT_LOGIC_CONFIG("appear.avatar", InitAvatar);

    return 0;
}

int ULogicConfig_Appear::InitAvatar(const nlohmann::json &data) {
    // TODO
    return 0;
}
