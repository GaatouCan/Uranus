#include "../include/game_world.h"
#include "../include/server_logic.h"
#include "../include/connection.h"
#include "../include/package_pool.h"
#include "../include/impl/package.h"

#include "../include/scene/scene_manager.h"
#include "../include/scene/main_scene.h"
#include "../include/reactor/global_queue.h"
#include "../include/config_manager.h"
#include "../include/login_authenticator.h"
#include "../include/protocol_route.h"

#include "../include/system/database/database_system.h"
#include "../include/system/event/event_system.h"
#include "../include/system/timer/timer_system.h"
#include "../include/system/plugin/plugin_system.h"
#include "../include/system/manager/manager_system.h"
#include "../include/system/command/command_system.h"

#include <ranges>
#include <random>


GameWorld::GameWorld()
    : mAcceptor(mContext),
      mModule(nullptr),
      mServer(nullptr),
      mDestroyer(nullptr),
      mFullTimer(mContext),
      bInited(false),
      bRunning(false) {

    mConfigManager = new ConfigManager();
    mSceneManager = new SceneManager(this);
    mGlobalQueue = new GlobalQueue(this);
    mLoginAuthenticator = new LoginAuthenticator(this);
    mProtocolRoute = new ProtocolRoute(this);

    // Create Sub System
    CreateSystem<DatabaseSystem>(2);
    CreateSystem<TimerSystem>(3);
    CreateSystem<CommandSystem>(4);
    CreateSystem<ManagerSystem>(9);
    CreateSystem<EventSystem>(10);
    CreateSystem<PluginSystem>(11);
}

GameWorld::~GameWorld() {
    Shutdown();

    while (!dest_priority_.empty()) {
        auto [priority, type] = dest_priority_.top();
        dest_priority_.pop();

        const auto iter = system_map_.find(type);
        if (iter == system_map_.end())
            continue;

        spdlog::info("{} Destroyed.", iter->second->GetSystemName());
        delete iter->second;

        system_map_.erase(iter);
    }

    // 正常情况下map应该是空了 但以防万一还是再遍历一次
    for (const auto sys: system_map_ | std::views::values) {
        delete sys;
    }

    delete mProtocolRoute;
    delete mGlobalQueue;
    delete mSceneManager;
    delete mLoginAuthenticator;
    delete mConfigManager;

    if (mServer && mDestroyer) {
        mDestroyer(mServer);
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(mModule);
#else
        dlclose(mModule);
#endif
        spdlog::info("Free Server Dynamic-link Library.");
    }

    spdlog::info("Game World Destroyed.");
    spdlog::drop_all();
}

GameWorld &GameWorld::Init(const std::string &path) {
    if (!LoadServerDLL(path)) {
        Shutdown();
        exit(-1);
    }

    mThreadID = std::this_thread::get_id();

    mConfigManager->Init();
    assert(mConfigManager->IsLoaded());

    const auto &config = GetServerConfig();

    PackagePool::LoadConfig(config);

    // 如果DLL未指定自定义数据包 则使用默认数据包
    if (!PackagePool::HasAssignedBuilder()) {
        Package::LoadConfig(config);
        PackagePool::SetPackageBuilder(&Package::CreatePackage);
        PackagePool::SetPackageInitializer(&Package::InitPackage);
    }

    mSceneManager->Init();
    mGlobalQueue->Init();
    mLoginAuthenticator->Init();

    while (!init_priority_.empty()) {
        auto [priority, type] = init_priority_.top();
        init_priority_.pop();

        const auto iter = system_map_.find(type);
        if (iter == system_map_.end())
            continue;

        iter->second->Init();
        spdlog::info("{} Initialized.", iter->second->GetSystemName());
    }

    bInited = true;

    return *this;
}

GameWorld &GameWorld::Run() {
    bRunning = true;

    asio::signal_set signals(mContext, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        Shutdown();
    });

    co_spawn(mContext, WaitForConnect(), detached);
    mContext.run();

    return *this;
}

GameWorld &GameWorld::Shutdown() {
    if (!bInited)
        return *this;

    if (!bRunning)
        return *this;

    spdlog::info("Server Shutting Down...");
    bRunning = false;

    if (!mContext.stopped())
        mContext.stop();

    mConnectionMap.clear();

    return *this;
}

void GameWorld::RemoveConnection(const std::string &key) {
    if (!bRunning)
        return;

    if (std::this_thread::get_id() != mThreadID) {
        co_spawn(mContext, [this, key]() mutable -> awaitable<void> {
            mConnectionMap.erase(key);
            co_return;
        }, detached);
        return;
    }
    mConnectionMap.erase(key);
}

ConfigManager *GameWorld::GetConfigManager() const {
    return mConfigManager;
}

SceneManager *GameWorld::GetSceneManager() const {
    return mSceneManager;
}

LoginAuthenticator * GameWorld::GetLoginAuthenticator() const {
    return mLoginAuthenticator;
}

ProtocolRoute * GameWorld::GetProtocolRoute() const {
    return mProtocolRoute;
}

GlobalQueue* GameWorld::GetGlobalQueue() const
{
    return mGlobalQueue;
}

ISubSystem *GameWorld::GetSystemByName(const std::string_view sys) const {
    if (const auto it = name_to_system_.find(sys); it != name_to_system_.end()) {
        return it->second;
    }
    return nullptr;
}

const YAML::Node &GameWorld::GetServerConfig() {
    if (!mConfigManager->IsLoaded()) {
        spdlog::critical("{} - ConfigSystem not loaded", __FUNCTION__);
        Shutdown();
        exit(-1);
    }

    return mConfigManager->GetServerConfig();
}

uint32_t GameWorld::GetServerID() {
    const auto &cfg = GetServerConfig();
    return cfg["server"]["cross_id"].as<uint32_t>();
}

bool GameWorld::LoadServerDLL(const std::string &path) {
#if defined(_WIN32) || defined(_WIN64)
    mModule = LoadLibrary(path.c_str());

    if (mModule == nullptr) {
        spdlog::error("Failed to load server dll: {}", path);
        return false;
    }

    const auto creator = reinterpret_cast<ServerCreator>(GetProcAddress(mModule, "CreateServer"));
    const auto destroyer = reinterpret_cast<ServerDestroyer>(GetProcAddress(mModule, "DestroyServer"));

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("Failed to load DLL function: {}", path);
        FreeLibrary(mModule);
        return false;
    }

#else
    mModule = dlopen(path.c_str(), RTLD_LAZY);

    if (mModule == nullptr) {
        spdlog::error("Failed to load server dll: {}", path);
        return false;
    }

    const auto creator = reinterpret_cast<ServerCreator>(dlsym(mModule, "CreateServer"));
    const auto destroyer = reinterpret_cast<ServerDestroyer>(dlsym(mModule, "DestroyServer"));

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("Failed to load DLL function: {}", path);
        dlclose(mModule);
        return false;
    }
#endif

    mServer = creator(this);
    mDestroyer = destroyer;

    mServer->InitGameWorld();

    mConfigManager->Abort();
    mProtocolRoute->AbortHandler();
    mLoginAuthenticator->AbortHandler();

    spdlog::info("Loaded Dynamic-link Library {} Success.", path);

    return true;
}

awaitable<void> GameWorld::WaitForConnect() {
    const auto &config = GetServerConfig();

    try {
        mAcceptor.open(tcp::v4());
        mAcceptor.bind({tcp::v4(), config["server"]["port"].as<uint16_t>()});
        mAcceptor.listen();

        spdlog::info("Waiting For Client To Connect - Server Port: {}", config["server"]["port"].as<uint16_t>());

        while (bRunning) {
            const auto scene = dynamic_cast<MainScene *>(mSceneManager->GetNextMainScene());
            if (scene == nullptr) {
                spdlog::critical("{} - Failed to get main scene.", __FUNCTION__);
                Shutdown();
                exit(-1);
            }

            if (auto socket = co_await mAcceptor.async_accept(scene->GetIOContext()); socket.is_open()) {
                const auto addr = socket.remote_endpoint().address();
                spdlog::info("New Connection From: {}", addr.to_string());

                if (!mLoginAuthenticator->VerifyAddress(socket.remote_endpoint().address())) {
                    socket.close();
                    spdlog::warn("Rejected Connection From: {}", addr.to_string());
                    continue;
                }

                std::string key;
                int count = 0;

                static std::random_device random_device;
                static std::mt19937 generator(random_device());
                static std::uniform_int_distribution distribution(100, 999);

                do {
                    key = fmt::format("{}-{}-{}", addr.to_string(), utils::UnixTime(), distribution(generator));
                    count++;
                } while (mConnectionMap.contains(key) && count < 3);

                if (count >= 3) {
                    socket.close();
                    spdlog::warn("Fail To Distribute Connection Key: {}", addr.to_string());
                    continue;
                }

                const auto conn = std::make_shared<Connection>(std::move(socket), scene);
                spdlog::info("Accept Connection From: {}", addr.to_string());

                conn->SetKey(key);

                mServer->SetConnectionCodec(conn);
                mServer->SetConnectionHandler(conn);

                conn->ConnectToClient();

                mConnectionMap[key] = conn;

                if (mConnectionMap.size() >= 1'000'000'000) {
                    mFullTimer.expires_after(10s);
                    co_await mFullTimer.async_wait();
                }
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
        Shutdown();
    }
}

void GameWorld::RemoveConnection(const std::string_view key) {
    if (!bRunning)
        return;

    if (std::this_thread::get_id() != mThreadID) {
        co_spawn(mContext, [this, key = std::string(key)]() mutable -> awaitable<void> {
            mConnectionMap.erase(key);
            co_return;
        }, detached);
        return;
    }
#if defined(_WIN32) || defined(_WIN64)
    mConnectionMap.erase(key);
#else
    mConnectionMap.erase(std::string(key));
#endif
}

asio::io_context &GameWorld::GetIOContext() {
    return mContext;
}

ThreadID GameWorld::GetThreadID() const {
    return mThreadID;
}

bool GameWorld::IsMainThread() const {
    return mThreadID == std::this_thread::get_id();
}
