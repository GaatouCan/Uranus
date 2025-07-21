#include "Config.h"
#include "Utils.h"

#include <fstream>
#include <spdlog/spdlog.h>


UConfig::UConfig() {

}

UConfig::~UConfig() {

}

void UConfig::SetYAMLPath(const std::string &path) {
    mYAMLPath = path;
}

void UConfig::SetJSONPath(const std::string &path) {
    mJSONPath = path;
}

const YAML::Node &UConfig::GetServerConfig() const {
    return mConfig;
}

int UConfig::GetServerID() const {
    if (state_ >= EModuleState::INITIALIZED)
        return mConfig["server"]["id"].as<int>();

    return -1;
}

void UConfig::Initial() {
    if (state_ != EModuleState::CREATED)
        return;

    SPDLOG_INFO("Using Server Configuration File: {}.", mYAMLPath + SERVER_CONFIG_FILE);

    mConfig = YAML::LoadFile(mYAMLPath + SERVER_CONFIG_FILE);

    assert(!mConfig.IsNull());
    SPDLOG_INFO("Checking Server Configuration File.");

    assert(!mConfig["server"].IsNull());
    assert(!mConfig["server"]["id"].IsNull());
    assert(!mConfig["server"]["port"].IsNull());
    assert(!mConfig["server"]["worker"].IsNull());
    assert(!mConfig["server"]["cross"].IsNull());

    assert(!mConfig["package"].IsNull());
    assert(!mConfig["package"]["magic"].IsNull());

    assert(!mConfig["service"].IsNull());

    SPDLOG_INFO("Server Configuration File Check Successfully.");

    const std::string jsonPath = !mJSONPath.empty() ? mJSONPath : mYAMLPath + SERVER_CONFIG_JSON;

    utils::TraverseFolder(jsonPath, [this, jsonPath](const std::filesystem::directory_entry &entry) {
        if (entry.path().extension().string() == ".json") {
            std::ifstream fs(entry.path());

            auto filepath = entry.path().string();
            filepath = filepath.substr(
                strlen(jsonPath.c_str()) + 1,
                filepath.length() - 6 - strlen(jsonPath.c_str()));

#ifdef WIN32
            filepath = utils::StringReplace(filepath, '\\', '.');
#elifdef __linux__
                filepath = utils::StringReplace(filepath, '/', '.');
#else
                filepath = utils::StringReplace(filepath, '/', '.');
#endif

            mJSONMap[filepath] = nlohmann::json::parse(fs);
            SPDLOG_INFO("\tLoaded {}.", filepath);
        }
    });

    SPDLOG_INFO("JSON Files Loaded Successfully.");
    state_ = EModuleState::INITIALIZED;
}

std::optional<nlohmann::json> UConfig::Find(const std::string &path) const {
    if (const auto it = mJSONMap.find(path); it != mJSONMap.end()) {
        return it->second;
    }
    return std::nullopt;
}
