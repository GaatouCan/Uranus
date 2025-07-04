#include "ConfigManager.h"

#include <ranges>
#include <spdlog/spdlog.h>

UConfigManager::UConfigManager()
    : bLoaded(false){
}

UConfigManager::~UConfigManager() {
    for (const auto &val : mConfigMap | std::views::values) {
        delete val;
    }
}

int UConfigManager::LoadConfig(UConfig *config) {
    if (bLoaded)
        return -1;

    for (const auto &[type, val] : mConfigMap) {
        if (val == nullptr) {
            SPDLOG_ERROR("{:<20} - LogicConfig[{}] Pointer Is Null", __FUNCTION__, type.name());
            return -2;
        }

        if (const auto res = val->Initial(config); res != 0) {
            SPDLOG_ERROR("{:<20} - LogicConfig[{}] Initialize Failed, Result = {}", __FUNCTION__, type.name(), res);
            return res;
        }
    }

    bLoaded = true;
    return 0;
}
