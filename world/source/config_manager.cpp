#include "../include/config_manager.h"
#include "../include/utils.h"

#include <fstream>
#include <spdlog/spdlog.h>


UConfigManager::UConfigManager()
    : logic_config_loader_(nullptr),
      logger_loader_(nullptr) {
}

UConfigManager::~UConfigManager() {
    for (const auto &loader: std::views::values(logic_config_map_)) {
        delete loader;
    }
}

void UConfigManager::Init() {
    spdlog::info("Using Server Configuration File: {}.", yaml_path_ + SERVER_CONFIG_FILE);

    cfg_ = YAML::LoadFile(yaml_path_ + SERVER_CONFIG_FILE);

    assert(!cfg_.IsNull());
    spdlog::info("Checking Server Configuration File.");

    assert(!cfg_["server"].IsNull());
    assert(!cfg_["server"]["port"].IsNull());
    assert(!cfg_["server"]["io_thread"].IsNull());
    assert(!cfg_["server"]["work_thread"].IsNull());
    assert(!cfg_["server"]["cross_id"].IsNull());

    assert(!cfg_["log_dir"].IsNull());

    assert(!cfg_["package"].IsNull());
    assert(!cfg_["package"]["magic"].IsNull());
    assert(!cfg_["package"]["version"].IsNull());
    assert(!cfg_["package"]["method"].IsNull());

    assert(!cfg_["package"]["pool"].IsNull());
    assert(!cfg_["package"]["pool"]["default_capacity"].IsNull());
    assert(!cfg_["package"]["pool"]["minimum_capacity"].IsNull());
    assert(!cfg_["package"]["pool"]["expanse_rate"].IsNull());
    assert(!cfg_["package"]["pool"]["expanse_scale"].IsNull());
    assert(!cfg_["package"]["pool"]["collect_rate"].IsNull());
    assert(!cfg_["package"]["pool"]["collect_scale"].IsNull());

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
        std::invoke(logger_loader_, cfg_);
    }

    loaded_ = true;
}

void UConfigManager::SetYAMLPath(const std::string &path) {
    yaml_path_ = path;
}

void UConfigManager::SetJSONPath(const std::string &path) {
    json_path_ = path;
}

void UConfigManager::SetLogicConfigLoader(const ALogicConfigLoader loader) {
    logic_config_loader_ = loader;
}

void UConfigManager::SetLoggerLoader(const ALoggerLoader loader) {
    logger_loader_ = loader;
}

void UConfigManager::Abort() const {
    assert(logic_config_loader_ != nullptr && logger_loader_ != nullptr);
}

bool UConfigManager::IsLoaded() const {
    return loaded_;
}

const YAML::Node &UConfigManager::GetServerConfig() const {
    return cfg_;
}

std::optional<nlohmann::json> UConfigManager::FindConfig(const std::string &path, const uint64_t id) const {
    if (const auto it = json_map_.find(path); it != json_map_.end()) {
        if (it->second.contains(std::to_string(id)))
            return it->second[std::to_string(id)];
    }

    return std::nullopt;
}

void UConfigManager::ReloadConfig() {
    cfg_ = YAML::LoadFile(yaml_path_ + SERVER_CONFIG_FILE);

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
