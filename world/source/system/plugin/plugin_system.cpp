#include "../../../include/system/plugin/plugin_system.h"
#include "../../../include/system/plugin/base_plugin.h"

#include <ranges>
#include <spdlog/spdlog.h>
#include <filesystem>

constexpr auto PLUGIN_DIRECTORY = "plugins";

PluginSystem::PluginSystem(GameWorld *world)
    : ISubSystem(world) {
}

PluginSystem::~PluginSystem() {
    for (const auto &[module, destroyer, plugin] : plugin_map_ | std::views::values) {
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

void PluginSystem::Init() {
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
            LoadPlugin(entry.path().string() + entry.path().filename().string());
        }
    }
}

PluginSystem::PluginNode PluginSystem::FindPlugin(const std::string &name) {
    std::shared_lock lock(mtx_);
    if (const auto it = plugin_map_.find(name); it != plugin_map_.end()) {
        return it->second;
    }
    return {nullptr, nullptr, nullptr};
}

bool PluginSystem::LoadPlugin(const std::string_view path) {
#if defined(_WIN32) || defined(_WIN64)
    const ModuleHandle hModule = LoadLibrary(path.data());

    if (hModule == nullptr) {
        spdlog::error("{} - Failed to load plugin {}", __FUNCTION__,  path.data());
        return false;
    }

    const auto creator = reinterpret_cast<PluginCreator>(GetProcAddress(hModule, "CreatePlugin"));
    const auto destroyer = reinterpret_cast<PluginDestroyer>(GetProcAddress(hModule, "DestroyPlugin"));

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
        std::shared_lock lock(mtx_);
        if (plugin_map_.contains(plugin->GetPluginName())) {
            spdlog::warn("{} - Plugin {} already exists", __FUNCTION__, plugin->GetPluginName());
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

    PluginNode node{ hModule, destroyer, plugin };

    std::unique_lock lock(mtx_);
    plugin_map_.insert_or_assign(plugin->GetPluginName(), node);
    spdlog::info("{} - Load {} Success.", __FUNCTION__, plugin->GetPluginName());

    return true;
}

bool PluginSystem::UnloadPlugin(const std::string &name) {
    const auto [module, destroyer, plugin] = FindPlugin(name);
    if (plugin == nullptr || destroyer == nullptr || module == nullptr) {
        spdlog::error("{} - Failed to find plugin {}", __FUNCTION__, name.data());
        return false;
    }

    {
        std::unique_lock lock(mtx_);
        plugin_map_.erase(name);
    }

    destroyer(plugin);
#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary(module);
#else
    dlclose(module);
#endif

    return true;
}
