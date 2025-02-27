#pragma once

#include "../../sub_system.h"
#include "../../utils.h"

#include <shared_mutex>


class IBasePlugin;

class BASE_API PluginSystem final : public ISubSystem {

    typedef IBasePlugin* (*PluginCreator)(PluginSystem *);
    typedef void (*PluginDestroyer)(IBasePlugin *);

    struct PluginNode {
        ModuleHandle    module;
        PluginDestroyer destroyer;
        IBasePlugin *   plugin;
    };

    std::unordered_map<std::string, PluginNode> plugin_map_;
    mutable std::shared_mutex mtx_;

public:
    explicit PluginSystem(GameWorld *world);
    ~PluginSystem() override;

    GET_SYSTEM_NAME(PluginSystem)

    void Init() override;

    PluginNode FindPlugin(const std::string &name);

    bool LoadPlugin(std::string_view path);
    bool UnloadPlugin(const std::string &name);
};
