#pragma once

#include "LogicConfig.h"

#include <typeindex>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

class UConfigManager;

typedef void(*ALogicConfigLoader)(UConfigManager *);
typedef void(*ALoggerLoader)(const YAML::Node&);

constexpr auto kServerConfigFile = "/server.yaml";
constexpr auto kServerConfigJSON = "/json";


class BASE_API UConfigManager final {

    std::string mYAMLPath;
    std::string mJSONPath;

    YAML::Node mConfig;
    std::unordered_map<std::string, nlohmann::json> mJSONConfigMap;

    std::unordered_map<std::type_index, std::vector<std::string>> mLogicLoadMap;
    std::unordered_map<std::type_index, ILogicConfig *> mLogicConfigMap;

    ALogicConfigLoader mLogicConfigLoader;
    ALoggerLoader mLoggerLoader;

    bool bLoaded = false;

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
        mLogicLoadMap[typeid(T)] = pathList;
        std::vector<nlohmann::json> configs;
        for (const auto &path : pathList) {
            if (const auto iter = mJSONConfigMap.find(path); iter != mJSONConfigMap.end()) {
                configs.push_back(iter->second);
            }
        }
        mLogicConfigMap.insert_or_assign(typeid(T), new T(configs));
    }

    template<LOGIC_CONFIG_TYPE T>
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
