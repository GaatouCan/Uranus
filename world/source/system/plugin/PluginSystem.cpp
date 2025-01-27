#include "../../../include/system/plugin/PluginSystem.h"
#include "../../../include/system/plugin/AbstractPlugin.h"

#include <ranges>
#include <spdlog/spdlog.h>
#include <filesystem>

constexpr auto kPluginDirectory = "plugins";

UPluginSystem::UPluginSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UPluginSystem::~UPluginSystem() {
    for (const auto &[module, destroyer, plugin] : mPluginMap | std::views::values) {
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

void UPluginSystem::Init() {
    if (!std::filesystem::exists(kPluginDirectory)) {
        try {
            std::filesystem::create_directory(kPluginDirectory);
        } catch (const std::exception &e) {
            spdlog::error("{} - Failed to create plugin directory {}", __FUNCTION__, e.what());
            return;
        }
    }

    for (const auto &entry : std::filesystem::directory_iterator(kPluginDirectory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".dll")
        {
            LoadPlugin(entry.path().string() + entry.path().filename().string());
        }
    }
}

UPluginSystem::FPluginNode UPluginSystem::FindPlugin(const std::string &name) {
    std::shared_lock lock(mPluginShared);
    if (const auto it = mPluginMap.find(name); it != mPluginMap.end()) {
        return it->second;
    }
    return {nullptr, nullptr, nullptr};
}

bool UPluginSystem::LoadPlugin(const std::string_view path) {
#if defined(_WIN32) || defined(_WIN64)
    const AModuleHandle hModule = LoadLibrary(path.data());
#else
    const AModuleHandle hModule = dlopen(path.data(), RTLD_LAZY);
#endif

    if (hModule == nullptr) {
        spdlog::error("{} - Failed to load plugin {}", __FUNCTION__,  path.data());
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)
    const auto creator = reinterpret_cast<APluginCreator>(GetProcAddress(hModule, "CreatePlugin"));
    const auto destroyer = reinterpret_cast<APluginDestroyer>(GetProcAddress(hModule, "DestroyPlugin"));
#else
    const auto creator = reinterpret_cast<APluginCreator>(dlsym(hModule, "CreatePlugin"));
    const auto destroyer = reinterpret_cast<APluginDestroyer>(dlsym(hModule, "DestroyPlugin"));
#endif

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("{} - Failed to find function {}", __FUNCTION__, path.data());
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(hModule);
#else
        dlclose(hModule);
#endif
        return false;
    }

    const auto plugin = creator(this);
    if (plugin == nullptr) {
        spdlog::error("{} - Failed to create plugin {}", __FUNCTION__, path.data());
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(hModule);
#else
        dlclose(hModule);
#endif
        return false;
    }

    {
        std::shared_lock lock(mPluginShared);
        if (mPluginMap.contains(plugin->GetPluginName())) {
            spdlog::warn("{} - Plugin {} already exists", __FUNCTION__, plugin->GetPluginName());
            destroyer(plugin);
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(hModule);
#else
            dlclose(hModule);
#endif
            return false;
        }
    }

    FPluginNode node{ hModule, destroyer, plugin };

    std::scoped_lock lock(mPluginMutex);
    mPluginMap.insert_or_assign(plugin->GetPluginName(), node);
    spdlog::info("{} - Load {} Success.", __FUNCTION__, plugin->GetPluginName());

    return true;
}

bool UPluginSystem::UnloadPlugin(const std::string &name) {
    const auto [module, destroyer, plugin] = FindPlugin(name);
    if (plugin == nullptr || destroyer == nullptr || module == nullptr) {
        spdlog::error("{} - Failed to find plugin {}", __FUNCTION__, name.data());
        return false;
    }

    {
        std::scoped_lock lock(mPluginMutex);
        mPluginMap.erase(name);
    }

    destroyer(plugin);
#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary(module);
#else
    dlclose(module);
#endif

    return true;
}
