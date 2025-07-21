#pragma once

#include "Module.h"

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <absl/container/flat_hash_map.h>


constexpr auto SERVER_CONFIG_FILE = "/server.yaml";
constexpr auto SERVER_CONFIG_JSON = "/json";


class BASE_API UConfig final : public IModuleBase {

    DECLARE_MODULE(UConfig)

protected:
    UConfig();

    void Initial() override;

public:
    ~UConfig() override;

    constexpr const char *GetModuleName() const override {
        return "Config Module";
    }

    void SetYAMLPath(const std::string &path);
    void SetJSONPath(const std::string &path);

    const YAML::Node &GetServerConfig() const;
    [[nodiscard]] int GetServerID() const;

    std::optional<nlohmann::json> Find(const std::string &path) const;

private:
    std::string mYAMLPath;
    std::string mJSONPath;

    YAML::Node mConfig;
    absl::flat_hash_map<std::string, nlohmann::json> mJSONMap;
};
