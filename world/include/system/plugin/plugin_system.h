#pragma once

#include "../../sub_system.h"
#include "../../utils.h"

#include <shared_mutex>


class IBasePlugin;

class BASE_API UPluginSystem final : public ISubSystem {

    typedef IBasePlugin* (*APluginCreator)(UPluginSystem *);
    typedef void (*APluginDestroyer)(IBasePlugin *);

    struct FPluginNode {
        AModuleHandle module;
        APluginDestroyer destroyer;
        IBasePlugin *plugin;
    };

    std::unordered_map<std::string, FPluginNode> pluginMap_;
    mutable std::shared_mutex mutex_;

public:
    explicit UPluginSystem(UGameWorld *world);
    ~UPluginSystem() override;

    GET_SYSTEM_NAME(PluginSystem)

    void init() override;

    FPluginNode findPlugin(const std::string &name);

    bool loadPlugin(std::string_view path);
    bool unloadPlugin(const std::string &name);
};
