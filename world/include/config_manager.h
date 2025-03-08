#pragma once

#include "logic_config.h"

#include <typeindex>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>


class UConfigManager;

typedef void(*ALogicConfigLoader)(UConfigManager *);
typedef void(*ALoggerLoader)(const YAML::Node&);

constexpr auto SERVER_CONFIG_FILE = "/server.yaml";
constexpr auto SERVER_CONFIG_JSON = "/json";


class BASE_API UConfigManager final {

    std::string yamlPath_;
    std::string jsonPath_;

    YAML::Node config_;
    std::unordered_map<std::string, nlohmann::json> jsonMap_;

    std::unordered_map<std::type_index, std::vector<std::string>> logicLoadMap_;
    std::unordered_map<std::type_index, ILogicConfig *> logicConfigMap_;

    ALogicConfigLoader logicConfigLoader_;
    ALoggerLoader loggerLoader_;

    bool loaded_ = false;

public:
    UConfigManager();
    ~UConfigManager();

    void init();

    void setYamlPath(const std::string &path);
    void setJsonPath(const std::string &path);

    void setLogicConfigLoader(ALogicConfigLoader loader);
    void setLoggerLoader(ALoggerLoader loader);

    void abort() const;

    template<CLogicConfigType T>
    void createLogicConfig(const std::vector<std::string> &path_list) {
        logicLoadMap_[typeid(T)] = path_list;
        std::vector<nlohmann::json> configs;
        for (const auto &path : path_list) {
            if (const auto iter = jsonMap_.find(path); iter != jsonMap_.end()) {
                configs.push_back(iter->second);
            }
        }
        logicConfigMap_.insert_or_assign(typeid(T), new T(configs));
    }

    template<CLogicConfigType T>
    T *findLogicConfig() {
        if (const auto iter = logicConfigMap_.find(typeid(T)); iter != logicConfigMap_.end()) {
            return dynamic_cast<T *>(iter->second);
        }
        return nullptr;
    }

    [[nodiscard]] bool loaded() const;
    const YAML::Node &getServerConfig() const;

    std::optional<nlohmann::json> find(const std::string &path, uint64_t id) const;

    void reloadConfig();
};

#define REGISTER_LOGIC_CONFIG(cfg, ...) \
    mgr->createLogicConfig<cfg>({__VA_ARGS__});
