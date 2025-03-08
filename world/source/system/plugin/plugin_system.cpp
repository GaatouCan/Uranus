#include "../../../include/system/plugin/plugin_system.h"
#include "../../../include/system/plugin/base_plugin.h"

#include <ranges>
#include <spdlog/spdlog.h>
#include <filesystem>

constexpr auto PLUGIN_DIRECTORY = "plugins";

UPluginSystem::UPluginSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UPluginSystem::~UPluginSystem() {
    for (const auto &[module, destroyer, plugin] : pluginMap_ | std::views::values) {
        if (destroyer) {
            destroyer(plugin);
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(module);
#else
            dlclose(module);
#endif
        }
    }
}

void UPluginSystem::init() {
    if (!std::filesystem::exists(PLUGIN_DIRECTORY)) {
        try {
            std::filesystem::create_directory(PLUGIN_DIRECTORY);
        } catch (const std::exception &e) {
            spdlog::error("{} - Failed to create plugin directory {}", __FUNCTION__, e.what());
            return;
        }
    }

    for (const auto &entry : std::filesystem::directory_iterator(PLUGIN_DIRECTORY))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".dll")
        {
            loadPlugin(entry.path().string() + entry.path().filename().string());
        }
    }
}

UPluginSystem::FPluginNode UPluginSystem::findPlugin(const std::string &name) {
    std::shared_lock lock(mutex_);
    if (const auto it = pluginMap_.find(name); it != pluginMap_.end()) {
        return it->second;
    }
    return {nullptr, nullptr, nullptr};
}

bool UPluginSystem::loadPlugin(const std::string_view path) {
#if defined(_WIN32) || defined(_WIN64)
    const AModuleHandle hModule = LoadLibrary(path.data());

    if (hModule == nullptr) {
        spdlog::error("{} - Failed to load plugin {}", __FUNCTION__,  path.data());
        return false;
    }

    const auto creator = reinterpret_cast<APluginCreator>(GetProcAddress(hModule, "CreatePlugin"));
    const auto destroyer = reinterpret_cast<APluginDestroyer>(GetProcAddress(hModule, "DestroyPlugin"));

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("{} - Failed to find function {}", __FUNCTION__, path.data());
        FreeLibrary(hModule);
        return false;
    }

    const auto plugin = creator(this);
    if (plugin == nullptr) {
        spdlog::error("{} - Failed to create plugin {}", __FUNCTION__, path.data());
        FreeLibrary(hModule);
        return false;
    }

    {
        std::shared_lock lock(mutex_);
        if (pluginMap_.contains(plugin->getPluginName())) {
            spdlog::warn("{} - Plugin {} already exists", __FUNCTION__, plugin->getPluginName());
            destroyer(plugin);
            FreeLibrary(hModule);
            return false;
        }
    }

#else
    const ModuleHandle hModule = dlopen(path.data(), RTLD_LAZY);

    if (hModule == nullptr) {
        spdlog::error("{} - Failed to load plugin {}", __FUNCTION__,  path.data());
        return false;
    }

    const auto creator = reinterpret_cast<PluginCreator>(dlsym(hModule, "CreatePlugin"));
    const auto destroyer = reinterpret_cast<PluginDestroyer>(dlsym(hModule, "DestroyPlugin"));

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("{} - Failed to find function {}", __FUNCTION__, path.data());
        dlclose(hModule);
        return false;
    }

    const auto plugin = creator(this);
    if (plugin == nullptr) {
        spdlog::error("{} - Failed to create plugin {}", __FUNCTION__, path.data());
        dlclose(hModule);
        return false;
    }

    {
        std::shared_lock lock(mtx_);
        if (plugin_map_.contains(plugin->GetPluginName())) {
            spdlog::warn("{} - Plugin {} already exists", __FUNCTION__, plugin->GetPluginName());
            destroyer(plugin);
            dlclose(hModule);
            return false;
        }
    }

#endif

    FPluginNode node{ hModule, destroyer, plugin };

    std::unique_lock lock(mutex_);
    pluginMap_.insert_or_assign(plugin->getPluginName(), node);
    spdlog::info("{} - Load {} Success.", __FUNCTION__, plugin->getPluginName());

    return true;
}

bool UPluginSystem::unloadPlugin(const std::string &name) {
    const auto [module, destroyer, plugin] = findPlugin(name);
    if (plugin == nullptr || destroyer == nullptr || module == nullptr) {
        spdlog::error("{} - Failed to find plugin {}", __FUNCTION__, name.data());
        return false;
    }

    {
        std::unique_lock lock(mutex_);
        pluginMap_.erase(name);
    }

    destroyer(plugin);
#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary(module);
#else
    dlclose(module);
#endif

    return true;
}
