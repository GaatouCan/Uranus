#include "../include/ConfigManager.h"
#include "../include/utils.h"

#include <fstream>
#include <spdlog/spdlog.h>


UConfigManager::UConfigManager()
    : mLogicConfigLoader(nullptr),
      mLoggerLoader(nullptr) {
}

UConfigManager::~UConfigManager() {
    for (const auto &loader: std::views::values(mLogicConfigMap)) {
        delete loader;
    }
}

void UConfigManager::Init() {
    spdlog::info("Using Server Configuration File: {}.", mYAMLPath + kServerConfigFile);

    mConfig = YAML::LoadFile(mYAMLPath + kServerConfigFile);

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

    const std::string jsonPath = !mJSONPath.empty() ? mJSONPath : mYAMLPath + kServerConfigJSON;

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

            mJSONConfigMap[filepath] = nlohmann::json::parse(fs);
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

void UConfigManager::SetYAMLPath(const std::string &path) {
    mYAMLPath = path;
}

void UConfigManager::SetJSONPath(const std::string &path) {
    mJSONPath = path;
}

void UConfigManager::SetLogicConfigLoader(const ALogicConfigLoader loader) {
    mLogicConfigLoader = loader;
}

void UConfigManager::SetLoggerLoader(const ALoggerLoader loader) {
    mLoggerLoader = loader;
}

void UConfigManager::Abort() const {
    assert(mLogicConfigLoader != nullptr && mLoggerLoader != nullptr);
}

bool UConfigManager::IsLoaded() const {
    return bLoaded;
}

const YAML::Node &UConfigManager::GetServerConfig() const {
    return mConfig;
}

std::optional<nlohmann::json> UConfigManager::FindConfig(const std::string &path, const uint64_t id) const {
    if (const auto it = mJSONConfigMap.find(path); it != mJSONConfigMap.end()) {
        if (it->second.contains(std::to_string(id)))
            return it->second[std::to_string(id)];
    }

    return std::nullopt;
}

void UConfigManager::ReloadConfig() {
    mConfig = YAML::LoadFile(mYAMLPath + kServerConfigFile);

    mJSONConfigMap.clear();
    const std::string jsonPath = !mJSONPath.empty() ? mJSONPath : mYAMLPath + kServerConfigJSON;

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

            mJSONConfigMap[filepath] = nlohmann::json::parse(fs);
            spdlog::info("\tLoaded {}.", filepath);
        }
    });

    for (auto &[type, cfg]: mLogicConfigMap) {
        if (auto vec = mLogicLoadMap.find(type); vec != mLogicLoadMap.end()) {
            std::vector<nlohmann::json> configs;
            for (const auto &path: vec->second) {
                if (const auto iter = mJSONConfigMap.find(path); iter != mJSONConfigMap.end()) {
                    configs.push_back(iter->second);
                }
            }
            cfg->OnReload(configs);
        }
    }
}
