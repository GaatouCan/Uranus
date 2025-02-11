#include "../include/config_manager.h"
#include "../include/utils.h"

#include <fstream>
#include <spdlog/spdlog.h>


ConfigManager::ConfigManager()
    : mLogicConfigLoader(nullptr),
      mLoggerLoader(nullptr) {
}

ConfigManager::~ConfigManager() {
    for (const auto &loader: std::views::values(mLogicConfigMap)) {
        delete loader;
    }
}

void ConfigManager::Init() {
    spdlog::info("Using Server Configuration File: {}.", mYAMLPath + SERVER_CONFIG_FILE);

    mConfig = YAML::LoadFile(mYAMLPath + SERVER_CONFIG_FILE);

    assert(!mConfig.IsNull());
    spdlog::info("Checking Server Configuration File.");

    assert(!mConfig["server"].IsNull());
    assert(!mConfig["server"]["port"].IsNull());
    assert(!mConfig["server"]["io_thread"].IsNull());
    assert(!mConfig["server"]["work_thread"].IsNull());
    assert(!mConfig["server"]["cross_id"].IsNull());

    assert(!mConfig["log_dir"].IsNull());

    assert(!mConfig["package"].IsNull());
    assert(!mConfig["package"]["magic"].IsNull());
    assert(!mConfig["package"]["version"].IsNull());
    assert(!mConfig["package"]["method"].IsNull());

    assert(!mConfig["package"]["pool"].IsNull());
    assert(!mConfig["package"]["pool"]["default_capacity"].IsNull());
    assert(!mConfig["package"]["pool"]["minimum_capacity"].IsNull());
    assert(!mConfig["package"]["pool"]["expanse_rate"].IsNull());
    assert(!mConfig["package"]["pool"]["expanse_scale"].IsNull());
    assert(!mConfig["package"]["pool"]["collect_rate"].IsNull());
    assert(!mConfig["package"]["pool"]["collect_scale"].IsNull());

    spdlog::info("Server Configuration File Check Successfully.");

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
            spdlog::info("\tLoaded {}.", filepath);
        }
    });

    if (mLogicConfigLoader) {
        std::invoke(mLogicConfigLoader, this);
    }

    if (mLoggerLoader) {
        std::invoke(mLoggerLoader, mConfig);
    }

    bLoaded = true;
}

void ConfigManager::SetYAMLPath(const std::string &path) {
    mYAMLPath = path;
}

void ConfigManager::SetJSONPath(const std::string &path) {
    mJSONPath = path;
}

void ConfigManager::SetLogicConfigLoader(const LogicConfigLoader loader) {
    mLogicConfigLoader = loader;
}

void ConfigManager::SetLoggerLoader(const LoggerLoader loader) {
    mLoggerLoader = loader;
}

void ConfigManager::Abort() const {
    assert(mLogicConfigLoader != nullptr && mLoggerLoader != nullptr);
}

bool ConfigManager::IsLoaded() const {
    return bLoaded;
}

const YAML::Node &ConfigManager::GetServerConfig() const {
    return mConfig;
}

std::optional<nlohmann::json> ConfigManager::FindConfig(const std::string &path, const uint64_t id) const {
    if (const auto it = mJSONMap.find(path); it != mJSONMap.end()) {
        if (it->second.contains(std::to_string(id)))
            return it->second[std::to_string(id)];
    }

    return std::nullopt;
}

void ConfigManager::ReloadConfig() {
    mConfig = YAML::LoadFile(mYAMLPath + SERVER_CONFIG_FILE);

    mJSONMap.clear();
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
            spdlog::info("\tLoaded {}.", filepath);
        }
    });

    for (auto &[type, cfg]: mLogicConfigMap) {
        if (auto vec = mLogicLoadMap.find(type); vec != mLogicLoadMap.end()) {
            std::vector<nlohmann::json> configs;
            for (const auto &path: vec->second) {
                if (const auto iter = mJSONMap.find(path); iter != mJSONMap.end()) {
                    configs.push_back(iter->second);
                }
            }
            cfg->OnReload(configs);
        }
    }
}
