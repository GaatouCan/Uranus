#pragma once

#include "logic_config.h"

#include <typeindex>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

class ConfigManager;

typedef void(*LogicConfigLoader)(ConfigManager *);
typedef void(*LoggerLoader)(const YAML::Node&);

constexpr auto SERVER_CONFIG_FILE = "/server.yaml";
constexpr auto SERVER_CONFIG_JSON = "/json";


class BASE_API ConfigManager final {

    std::string yaml_path_;
    std::string json_path_;

    YAML::Node config_;
    std::unordered_map<std::string, nlohmann::json> json_map_;

    std::unordered_map<std::type_index, std::vector<std::string>> logic_load_map_;
    std::unordered_map<std::type_index, ILogicConfig *> logic_config_map_;

    LogicConfigLoader logic_config_loader_;
    LoggerLoader logger_loader_;

    bool loaded_ = false;

public:
    ConfigManager();
    ~ConfigManager();

    void Init();

    void SetYAMLPath(const std::string &path);
    void SetJSONPath(const std::string &path);

    void SetLogicConfigLoader(LogicConfigLoader loader);
    void SetLoggerLoader(LoggerLoader loader);

    void Abort() const;

    template<LogicConfigType T>
    void CreateLogicConfig(const std::vector<std::string> &path_list) {
        logic_load_map_[typeid(T)] = path_list;
        std::vector<nlohmann::json> configs;
        for (const auto &path : path_list) {
            if (const auto iter = json_map_.find(path); iter != json_map_.end()) {
                configs.push_back(iter->second);
            }
        }
        logic_config_map_.insert_or_assign(typeid(T), new T(configs));
    }

    template<LogicConfigType T>
    T *FindLogicConfig() {
        if (const auto iter = logic_config_map_.find(typeid(T)); iter != logic_config_map_.end()) {
            return dynamic_cast<T *>(iter->second);
        }
        return nullptr;
    }

    [[nodiscard]] bool IsLoaded() const;
    const YAML::Node &GetServerConfig() const;

    std::optional<nlohmann::json> FindConfig(const std::string &path, uint64_t id) const;

    void ReloadConfig();
};

#define REGISTER_LOGIC_CONFIG(cfg, ...) \
    mgr->CreateLogicConfig<cfg>({__VA_ARGS__});
