#pragma once

#include "../../SubSystem.h"
#include "../../utils.h"

#include <shared_mutex>


class IAbstractPlugin;

class BASE_API PluginSystem final : public ISubSystem {

    typedef IAbstractPlugin* (*PluginCreator)(PluginSystem *);
    typedef void (*PluginDestroyer)(IAbstractPlugin *);

    struct PluginNode {
        ModuleHandle module;
        PluginDestroyer destroyer;
        IAbstractPlugin *plugin;
    };

    std::unordered_map<std::string, PluginNode> plugin_map_;
    mutable std::shared_mutex mutex_;

public:
    explicit PluginSystem(GameWorld *world);
    ~PluginSystem() override;

    GET_SYSTEM_NAME(UPluginSystem)

    void Init() override;

    PluginNode FindPlugin(const std::string &name);

    bool LoadPlugin(std::string_view path);
    bool UnloadPlugin(const std::string &name);
};
