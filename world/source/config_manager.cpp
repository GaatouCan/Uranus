#include "../include/config_manager.h"
#include "../include/utils.h"

#include <fstream>
#include <spdlog/spdlog.h>


UConfigManager::UConfigManager()
    : logicConfigLoader_(nullptr),
      loggerLoader_(nullptr) {
}

UConfigManager::~UConfigManager() {
    for (const auto &loader: std::views::values(logicConfigMap_)) {
        delete loader;
    }
}

void UConfigManager::init() {
    spdlog::info("Using Server Configuration File: {}.", yamlPath_ + SERVER_CONFIG_FILE);

    config_ = YAML::LoadFile(yamlPath_ + SERVER_CONFIG_FILE);

    assert(!config_.IsNull());
    spdlog::info("Checking Server Configuration File.");

    assert(!config_["server"].IsNull());
    assert(!config_["server"]["port"].IsNull());
    assert(!config_["server"]["io_thread"].IsNull());
    assert(!config_["server"]["work_thread"].IsNull());
    assert(!config_["server"]["cross_id"].IsNull());

    assert(!config_["log_dir"].IsNull());

    assert(!config_["package"].IsNull());
    assert(!config_["package"]["magic"].IsNull());
    assert(!config_["package"]["version"].IsNull());
    assert(!config_["package"]["method"].IsNull());

    assert(!config_["package"]["pool"].IsNull());
    assert(!config_["package"]["pool"]["default_capacity"].IsNull());
    assert(!config_["package"]["pool"]["minimum_capacity"].IsNull());
    assert(!config_["package"]["pool"]["expanse_rate"].IsNull());
    assert(!config_["package"]["pool"]["expanse_scale"].IsNull());
    assert(!config_["package"]["pool"]["collect_rate"].IsNull());
    assert(!config_["package"]["pool"]["collect_scale"].IsNull());

    spdlog::info("Server Configuration File Check Successfully.");

    const std::string jsonPath = !jsonPath_.empty() ? jsonPath_ : yamlPath_ + SERVER_CONFIG_JSON;

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

            jsonMap_[filepath] = nlohmann::json::parse(fs);
            spdlog::info("\tLoaded {}.", filepath);
        }
    });

    if (logicConfigLoader_) {
        std::invoke(logicConfigLoader_, this);
    }

    if (loggerLoader_) {
        std::invoke(loggerLoader_, config_);
    }

    loaded_ = true;
}

void UConfigManager::setYamlPath(const std::string &path) {
    yamlPath_ = path;
}

void UConfigManager::setJsonPath(const std::string &path) {
    jsonPath_ = path;
}

void UConfigManager::setLogicConfigLoader(const ALogicConfigLoader loader) {
    logicConfigLoader_ = loader;
}

void UConfigManager::setLoggerLoader(const ALoggerLoader loader) {
    loggerLoader_ = loader;
}

void UConfigManager::abort() const {
    assert(logicConfigLoader_ != nullptr && loggerLoader_ != nullptr);
}

bool UConfigManager::loaded() const {
    return loaded_;
}

const YAML::Node &UConfigManager::getServerConfig() const {
    return config_;
}

std::optional<nlohmann::json> UConfigManager::find(const std::string &path, const uint64_t id) const {
    if (const auto it = jsonMap_.find(path); it != jsonMap_.end()) {
        if (it->second.contains(std::to_string(id)))
            return it->second[std::to_string(id)];
    }

    return std::nullopt;
}

void UConfigManager::reloadConfig() {
    config_ = YAML::LoadFile(yamlPath_ + SERVER_CONFIG_FILE);

    jsonMap_.clear();
    const std::string jsonPath = !jsonPath_.empty() ? jsonPath_ : yamlPath_ + SERVER_CONFIG_JSON;

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

            jsonMap_[filepath] = nlohmann::json::parse(fs);
            spdlog::info("\tLoaded {}.", filepath);
        }
    });

    for (auto &[type, cfg]: logicConfigMap_) {
        if (auto vec = logicLoadMap_.find(type); vec != logicLoadMap_.end()) {
            std::vector<nlohmann::json> configs;
            for (const auto &path: vec->second) {
                if (const auto iter = jsonMap_.find(path); iter != jsonMap_.end()) {
                    configs.push_back(iter->second);
                }
            }
            cfg->onReload(configs);
        }
    }
}
