#pragma once

#include "../../SubSystem.h"
#include "../../utils.h"

#include <shared_mutex>

class IAbstractPlugin;

class BASE_API UPluginSystem final : public ISubSystem {

    typedef IAbstractPlugin* (*APluginCreator)(UPluginSystem *);
    typedef void (*APluginDestroyer)(IAbstractPlugin *);

    struct FPluginNode {
        AModuleHandle module;
        APluginDestroyer destroyer;
        IAbstractPlugin *plugin;
    };

    std::unordered_map<std::string, FPluginNode> mPluginMap;
    std::mutex mPluginMutex;
    mutable std::shared_mutex mPluginShared;

public:
    explicit UPluginSystem(UGameWorld *world);
    ~UPluginSystem() override;

    GET_SYSTEM_NAME(UPluginSystem)

    void Init() override;

    FPluginNode FindPlugin(const std::string &name);

    bool LoadPlugin(std::string_view path);
    bool UnloadPlugin(const std::string &name);
};
