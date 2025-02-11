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

    std::string mYAMLPath;
    std::string mJSONPath;

    YAML::Node mConfig;
    std::unordered_map<std::string, nlohmann::json> mJSONMap;

    std::unordered_map<std::type_index, std::vector<std::string>> mLogicLoadMap;
    std::unordered_map<std::type_index, ILogicConfig *> mLogicConfigMap;

    LogicConfigLoader mLogicConfigLoader;
    LoggerLoader mLoggerLoader;

    bool bLoaded = false;

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
        mLogicLoadMap[typeid(T)] = path_list;
        std::vector<nlohmann::json> configs;
        for (const auto &path : path_list) {
            if (const auto iter = mJSONMap.find(path); iter != mJSONMap.end()) {
                configs.push_back(iter->second);
            }
        }
        mLogicConfigMap.insert_or_assign(typeid(T), new T(configs));
    }

    template<LogicConfigType T>
    T *FindLogicConfig() {
        if (const auto iter = mLogicConfigMap.find(typeid(T)); iter != mLogicConfigMap.end()) {
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
