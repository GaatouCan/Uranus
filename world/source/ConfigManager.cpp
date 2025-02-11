#include "../include/ConfigManager.h"
#include "../include/utils.h"

#include <fstream>
#include <spdlog/spdlog.h>


ConfigManager::ConfigManager()
    : logic_config_loader_(nullptr),
      logger_loader_(nullptr) {
}

ConfigManager::~ConfigManager() {
    for (const auto &loader: std::views::values(logic_config_map_)) {
        delete loader;
    }
}

void ConfigManager::Init() {
    spdlog::info("Using Server Configuration File: {}.", yaml_path_ + SERVER_CONFIG_FILE);

    config_ = YAML::LoadFile(yaml_path_ + SERVER_CONFIG_FILE);

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

    const std::string jsonPath = !json_path_.empty() ? json_path_ : yaml_path_ + SERVER_CONFIG_JSON;

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

            json_map_[filepath] = nlohmann::json::parse(fs);
            spdlog::info("\tLoaded {}.", filepath);
        }
    });

    if (logic_config_loader_) {
        std::invoke(logic_config_loader_, this);
    }

    if (logger_loader_) {
        std::invoke(logger_loader_, config_);
    }

    loaded_ = true;
}

void ConfigManager::SetYAMLPath(const std::string &path) {
    yaml_path_ = path;
}

void ConfigManager::SetJSONPath(const std::string &path) {
    json_path_ = path;
}

void ConfigManager::SetLogicConfigLoader(const LogicConfigLoader loader) {
    logic_config_loader_ = loader;
}

void ConfigManager::SetLoggerLoader(const LoggerLoader loader) {
    logger_loader_ = loader;
}

void ConfigManager::Abort() const {
    assert(logic_config_loader_ != nullptr && logger_loader_ != nullptr);
}

bool ConfigManager::IsLoaded() const {
    return loaded_;
}

const YAML::Node &ConfigManager::GetServerConfig() const {
    return config_;
}

std::optional<nlohmann::json> ConfigManager::FindConfig(const std::string &path, const uint64_t id) const {
    if (const auto it = json_map_.find(path); it != json_map_.end()) {
        if (it->second.contains(std::to_string(id)))
            return it->second[std::to_string(id)];
    }

    return std::nullopt;
}

void ConfigManager::ReloadConfig() {
    config_ = YAML::LoadFile(yaml_path_ + SERVER_CONFIG_FILE);

    json_map_.clear();
    const std::string jsonPath = !json_path_.empty() ? json_path_ : yaml_path_ + SERVER_CONFIG_JSON;

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

            json_map_[filepath] = nlohmann::json::parse(fs);
            spdlog::info("\tLoaded {}.", filepath);
        }
    });

    for (auto &[type, cfg]: logic_config_map_) {
        if (auto vec = logic_load_map_.find(type); vec != logic_load_map_.end()) {
            std::vector<nlohmann::json> configs;
            for (const auto &path: vec->second) {
                if (const auto iter = json_map_.find(path); iter != json_map_.end()) {
                    configs.push_back(iter->second);
                }
            }
            cfg->OnReload(configs);
        }
    }
}
