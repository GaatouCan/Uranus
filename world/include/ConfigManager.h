#pragma once

#include "LogicConfig.h"

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

    void Init();

    void SetYAMLPath(const std::string &path);
    void SetJSONPath(const std::string &path);

    void SetLogicConfigLoader(ALogicConfigLoader loader);
    void SetLoggerLoader(ALoggerLoader loader);

    void Abort() const;

    template<LOGIC_CONFIG_TYPE T>
    void CreateLogicConfig(const std::vector<std::string> &pathList) {
        logicLoadMap_[typeid(T)] = pathList;
        std::vector<nlohmann::json> configs;
        for (const auto &path : pathList) {
            if (const auto iter = jsonMap_.find(path); iter != jsonMap_.end()) {
                configs.push_back(iter->second);
            }
        }
        logicConfigMap_.insert_or_assign(typeid(T), new T(configs));
    }

    template<LOGIC_CONFIG_TYPE T>
    T *FindLogicConfig() {
        if (const auto iter = logicConfigMap_.find(typeid(T)); iter != logicConfigMap_.end()) {
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
