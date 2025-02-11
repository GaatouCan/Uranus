#include "../include/GameWorld.h"
#include "../include/ServerLogic.h"
#include "../include/impl/Package.h"
#include "../include/Connection.h"
#include "../include/PackagePool.h"
#include "../include/impl/PackageCodec.h"

#include "../include/scene/SceneManager.h"
#include "../include/scene/MainScene.h"
#include "../include/reactor/GlobalQueue.h"
#include "../include/ConfigManager.h"
#include "../include/LoginAuthenticator.h"
#include "../include/ProtocolRoute.h"

#include "../include/system/event/EventSystem.h"
#include "../include/system/timer/TimerSystem.h"
#include "../include/system/plugin/PluginSystem.h"
#include "../include/system/manager/ManagerSystem.h"

#include <ranges>
#include <random>


GameWorld::GameWorld()
    : acceptor(ctx_),
      module_(nullptr),
      server_(nullptr),
      server_destroyer_(nullptr),
      full_timer_(ctx_),
      inited_(false),
      running_(false) {

    config_manager_ = new ConfigManager();
    scene_manager_ = new SceneManager(this);
    global_queue_ = new GlobalQueue(this);
    login_authenticator_ = new LoginAuthenticator(this);
    protocol_route_ = new ProtocolRoute(this);

    // Create Sub System
    CreateSystem<TimerSystem>(3);
    CreateSystem<ManagerSystem>(4);
    CreateSystem<EventSystem>(5);
    CreateSystem<PluginSystem>(6);
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

    delete protocol_route_;
    delete global_queue_;
    delete scene_manager_;
    delete login_authenticator_;
    delete config_manager_;

    if (server_ && server_destroyer_) {
        server_destroyer_(server_);
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(module_);
#else
        dlclose(mModule);
#endif
        spdlog::info("Free Server DLL.");
    }

    spdlog::info("Game World Destroyed.");
    spdlog::drop_all();
}

GameWorld &GameWorld::Init(const std::string &dll_path) {
    if (!LoadServerDLL(dll_path)) {
        Shutdown();
        exit(-1);
    }

    world_thread_id_ = std::this_thread::get_id();

    config_manager_->Init();
    assert(config_manager_->IsLoaded());

    const auto &config = GetServerConfig();

    Package::LoadConfig(config);
    PackagePool::LoadConfig(config);

    PackagePool::SetPackageBuilder(&Package::CreatePackage);
    PackagePool::SetPackageInitializer(&Package::InitPackage);

    scene_manager_->Init();
    global_queue_->Init();
    login_authenticator_->Init();

    while (!init_priority_.empty()) {
        auto [priority, type] = init_priority_.top();
        init_priority_.pop();

        const auto iter = system_map_.find(type);
        if (iter == system_map_.end())
            continue;

        iter->second->Init();
        spdlog::info("{} Initialized.", iter->second->GetSystemName());
    }

    inited_ = true;

    return *this;
}

GameWorld &GameWorld::Run() {
    running_ = true;

    asio::signal_set signals(ctx_, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        Shutdown();
    });

    co_spawn(ctx_, WaitForConnect(), detached);
    ctx_.run();

    return *this;
}

GameWorld &GameWorld::Shutdown() {
    if (!inited_)
        return *this;

    if (!running_)
        return *this;

    spdlog::info("Server Shutting Down...");
    running_ = false;

    if (!ctx_.stopped())
        ctx_.stop();

    connection_map_.clear();

    return *this;
}

void GameWorld::RemoveConnection(const std::string &key) {
    if (!running_)
        return;

    if (std::this_thread::get_id() != world_thread_id_) {
        co_spawn(ctx_, [this, key]() mutable -> awaitable<void> {
            connection_map_.erase(key);
            co_return;
        }, detached);
        return;
    }
    connection_map_.erase(key);
}

ConfigManager *GameWorld::GetConfigManager() const {
    return config_manager_;
}

SceneManager *GameWorld::GetSceneManager() const {
    return scene_manager_;
}

LoginAuthenticator * GameWorld::GetLoginAuthenticator() const {
    return login_authenticator_;
}

ProtocolRoute * GameWorld::GetProtocolRoute() const {
    return protocol_route_;
}

GlobalQueue* GameWorld::GetGlobalQueue() const
{
    return global_queue_;
}

ISubSystem *GameWorld::GetSystemByName(const std::string_view sys) const {
    if (const auto it = name_to_system_.find(sys); it != name_to_system_.end()) {
        return it->second;
    }
    return nullptr;
}

const YAML::Node &GameWorld::GetServerConfig() {
    if (!config_manager_->IsLoaded()) {
        spdlog::critical("{} - ConfigSystem not loaded", __FUNCTION__);
        Shutdown();
        exit(-1);
    }

    return config_manager_->GetServerConfig();
}

uint32_t GameWorld::GetServerID() {
    const auto &cfg = GetServerConfig();
    return cfg["server"]["cross_id"].as<uint32_t>();
}

bool GameWorld::LoadServerDLL(const std::string &path) {
#if defined(_WIN32) || defined(_WIN64)
    module_ = LoadLibrary(path.c_str());
#else
    mModule = dlopen(path.c_str(), RTLD_LAZY);
#endif

    if (module_ == nullptr) {
        spdlog::error("Failed to load server dll: {}", path);
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)
    const auto creator = reinterpret_cast<ServerCreator>(GetProcAddress(module_, "CreateServer"));
    const auto destroyer = reinterpret_cast<ServerDestroyer>(GetProcAddress(module_, "DestroyServer"));
#else
    const auto creator = reinterpret_cast<AServerCreator>(dlsym(mModule, "CreateServer"));
    const auto destroyer = reinterpret_cast<AServerDestroyer>(dlsym(mModule, "DestroyServer"));
#endif

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("Failed to load DLL function: {}", path);

#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(module_);
#else
        dlclose(mModule);
#endif

        return false;
    }

    server_ = creator(this);
    server_destroyer_ = destroyer;

    server_->InitGameWorld();

    config_manager_->Abort();
    protocol_route_->AbortHandler();
    login_authenticator_->AbortHandler();

    spdlog::info("Loaded DLL {} Success.", path);

    return true;
}

awaitable<void> GameWorld::WaitForConnect() {
    const auto &config = GetServerConfig();

    try {
        acceptor.open(tcp::v4());
        acceptor.bind({tcp::v4(), config["server"]["port"].as<uint16_t>()});
        acceptor.listen();

        spdlog::info("Waiting For Client To Connect - Server Port: {}", config["server"]["port"].as<uint16_t>());

        while (running_) {
            const auto scene = dynamic_cast<MainScene *>(scene_manager_->GetNextMainScene());
            if (scene == nullptr) {
                spdlog::critical("{} - Failed to get main scene.", __FUNCTION__);
                Shutdown();
                exit(-1);
            }

            if (auto socket = co_await acceptor.async_accept(scene->GetIOContext()); socket.is_open()) {
                const auto addr = socket.remote_endpoint().address();
                spdlog::info("New Connection From: {}", addr.to_string());

                if (!login_authenticator_->VerifyAddress(socket.remote_endpoint().address())) {
                    socket.close();
                    spdlog::warn("Rejected Connection From: {}", addr.to_string());
                    continue;
                }

                std::string key;
                int count = 0;

                static std::random_device sRandomDevice;
                static std::mt19937 sGenerator(sRandomDevice());
                static std::uniform_int_distribution sDistribution(100, 999);

                do {
                    key = fmt::format("{}-{}-{}", addr.to_string(), utils::UnixTime(), sDistribution(sGenerator));
                    count++;
                } while (connection_map_.contains(key) && count < 3);

                if (count >= 3) {
                    socket.close();
                    spdlog::warn("Fail To Distribute Connection Key: {}", addr.to_string());
                    continue;
                }

                const auto conn = std::make_shared<Connection>(std::move(socket), scene);
                spdlog::info("Accept Connection From: {}", addr.to_string());

                conn->SetKey(key);
                conn->SetPackageCodec<PackageCodec>();

                server_->SetConnectionHandler(conn);

                conn->ConnectToClient();

                connection_map_[key] = conn;

                if (connection_map_.size() >= 1'000'000'000) {
                    full_timer_.expires_after(10s);
                    co_await full_timer_.async_wait();
                }
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
        Shutdown();
    }
}

void GameWorld::RemoveConnection(const std::string_view key) {
    if (std::this_thread::get_id() != world_thread_id_) {
        co_spawn(ctx_, [this, key]() mutable -> awaitable<void> {
            connection_map_.erase(key);
            co_return;
        }, detached);
        return;
    }
    connection_map_.erase(key);
}

asio::io_context &GameWorld::GetIOContext() {
    return ctx_;
}

ThreadID GameWorld::GetThreadID() const {
    return world_thread_id_;
}

bool GameWorld::IsMainThread() const {
    return world_thread_id_ == std::this_thread::get_id();
}
