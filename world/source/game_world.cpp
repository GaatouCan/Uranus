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


UGameWorld::UGameWorld()
    : acceptor_(ctx_),
      module_(nullptr),
      server_(nullptr),
      destroyer_(nullptr),
      full_timer_(ctx_),
      inited_(false),
      running_(false) {

    cfg_mgr_                = new UConfigManager();
    scene_mgr_              = new USceneManager(this);
    global_queue_           = new UGlobalQueue(this);
    login_authenticator_    = new ULoginAuthenticator(this);
    proto_route_            = new UProtocolRoute(this);

    // Create Sub System
    CreateSystem<UDatabaseSystem>(2);
    CreateSystem<UTimerSystem>(3);
    CreateSystem<UCommandSystem>(4);
    CreateSystem<UManagerSystem>(9);
    CreateSystem<UEventSystem>(10);
    CreateSystem<UPluginSystem>(11);
}

UGameWorld::~UGameWorld() {
    Shutdown();

    while (!dest_priority_.empty()) {
        auto [priority, type] = dest_priority_.top();
        dest_priority_.pop();

        const auto iter = sys_map_.find(type);
        if (iter == sys_map_.end())
            continue;

        spdlog::info("{} Destroyed.", iter->second->GetSystemName());
        delete iter->second;

        sys_map_.erase(iter);
    }

    // 正常情况下map应该是空了 但以防万一还是再遍历一次
    for (const auto sys: sys_map_ | std::views::values) {
        delete sys;
    }

    delete proto_route_;
    delete global_queue_;
    delete scene_mgr_;
    delete login_authenticator_;
    delete cfg_mgr_;

    if (server_ && destroyer_) {
        destroyer_(server_);
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(module_);
#else
        dlclose(module_);
#endif
        spdlog::info("Free Server Dynamic-link Library.");
    }

    spdlog::info("Game World Destroyed.");
    spdlog::drop_all();
}

UGameWorld &UGameWorld::Init(const std::string &path) {
    if (!LoadServerDLL(path)) {
        Shutdown();
        exit(-1);
    }

    tid_ = std::this_thread::get_id();

    cfg_mgr_->Init();
    assert(cfg_mgr_->IsLoaded());

    const auto &config = GetServerConfig();

    PackagePool::LoadConfig(config);

    // 如果DLL未指定自定义数据包 则使用默认数据包
    if (!PackagePool::HasAssignedBuilder()) {
        FPackage::LoadConfig(config);
        PackagePool::SetPackageBuilder(&FPackage::CreatePackage);
        PackagePool::SetPackageInitializer(&FPackage::InitPackage);
    }

    scene_mgr_->Init();
    global_queue_->Init();
    login_authenticator_->Init();

    while (!init_priority_.empty()) {
        auto [priority, type] = init_priority_.top();
        init_priority_.pop();

        const auto iter = sys_map_.find(type);
        if (iter == sys_map_.end())
            continue;

        iter->second->Init();
        spdlog::info("{} Initialized.", iter->second->GetSystemName());
    }

    inited_ = true;

    return *this;
}

UGameWorld &UGameWorld::Run() {
    running_ = true;

    asio::signal_set signals(ctx_, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        Shutdown();
    });

    co_spawn(ctx_, WaitForConnect(), detached);
    ctx_.run();

    return *this;
}

UGameWorld &UGameWorld::Shutdown() {
    if (!inited_)
        return *this;

    if (!running_)
        return *this;

    spdlog::info("Server Shutting Down...");
    running_ = false;

    if (!ctx_.stopped())
        ctx_.stop();

    conn_map_.clear();

    return *this;
}

void UGameWorld::ForceDisconnectAll() {
    if (!running_)
        return;

    if (std::this_thread::get_id() != tid_) {
        co_spawn(ctx_, [this]() mutable -> awaitable<void> {
            for (const auto &conn : conn_map_ | std::views::values) {
                conn->Disconnect();
            }
            conn_map_.clear();
            co_return;
        }, detached);
        return;
    }

    for (const auto &conn : conn_map_ | std::views::values) {
        conn->Disconnect();
    }
    conn_map_.clear();
}

void UGameWorld::RemoveConnection(const std::string &key) {
    if (!running_)
        return;

    if (std::this_thread::get_id() != tid_) {
        co_spawn(ctx_, [this, key]() mutable -> awaitable<void> {
            conn_map_.erase(key);
            co_return;
        }, detached);
        return;
    }
    conn_map_.erase(key);
}

UConfigManager *UGameWorld::GetConfigManager() const {
    return cfg_mgr_;
}

USceneManager *UGameWorld::GetSceneManager() const {
    return scene_mgr_;
}

ULoginAuthenticator * UGameWorld::GetLoginAuthenticator() const {
    return login_authenticator_;
}

UProtocolRoute * UGameWorld::GetProtocolRoute() const {
    return proto_route_;
}

UGlobalQueue* UGameWorld::GetGlobalQueue() const
{
    return global_queue_;
}

ISubSystem *UGameWorld::GetSystemByName(const std::string_view sys) const {
    if (const auto it = name_to_sys_.find(sys); it != name_to_sys_.end()) {
        return it->second;
    }
    return nullptr;
}

const YAML::Node &UGameWorld::GetServerConfig() {
    if (!cfg_mgr_->IsLoaded()) {
        spdlog::critical("{} - ConfigSystem not loaded", __FUNCTION__);
        Shutdown();
        exit(-1);
    }

    return cfg_mgr_->GetServerConfig();
}

int32_t UGameWorld::GetServerID() {
    const auto &cfg = GetServerConfig();
    return cfg["server"]["cross_id"].as<int32_t>();
}

bool UGameWorld::LoadServerDLL(const std::string &path) {
#if defined(_WIN32) || defined(_WIN64)
    module_ = LoadLibrary(path.c_str());

    if (module_ == nullptr) {
        spdlog::error("Failed to load server dll: {}", path);
        return false;
    }

    const auto creator = reinterpret_cast<AServerCreator>(GetProcAddress(module_, "CreateServer"));
    const auto destroyer = reinterpret_cast<AServerDestroyer>(GetProcAddress(module_, "DestroyServer"));

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("Failed to load DLL function: {}", path);
        FreeLibrary(module_);
        return false;
    }

#else
    module_ = dlopen(path.c_str(), RTLD_LAZY);

    if (module_== nullptr) {
        spdlog::error("Failed to load server dll: {}", path);
        return false;
    }

    const auto creator = reinterpret_cast<ServerCreator>(dlsym(module_, "CreateServer"));
    const auto destroyer = reinterpret_cast<ServerDestroyer>(dlsym(module_, "DestroyServer"));

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("Failed to load DLL function: {}", path);
        dlclose(module_);
        return false;
    }
#endif

    server_ = creator(this);
    destroyer_ = destroyer;

    server_->InitGameWorld();

    cfg_mgr_->Abort();
    proto_route_->AbortHandler();
    login_authenticator_->AbortHandler();

    spdlog::info("Loaded Dynamic-link Library {} Success.", path);

    return true;
}

awaitable<void> UGameWorld::WaitForConnect() {
    const auto &config = GetServerConfig();

    try {
        acceptor_.open(tcp::v4());
        acceptor_.bind({tcp::v4(), config["server"]["port"].as<uint16_t>()});
        acceptor_.listen();

        spdlog::info("Waiting For Client To Connect - Server Port: {}", config["server"]["port"].as<uint16_t>());

        while (running_) {
            const auto scene = dynamic_cast<UMainScene *>(scene_mgr_->GetNextMainScene());
            if (scene == nullptr) {
                spdlog::critical("{} - Failed to get main scene.", __FUNCTION__);
                Shutdown();
                exit(-1);
            }

            if (auto socket = co_await acceptor_.async_accept(scene->GetIOContext()); socket.is_open()) {
                const auto addr = socket.remote_endpoint().address();
                spdlog::info("New Connection From: {}", addr.to_string());

                if (!login_authenticator_->VerifyAddress(socket.remote_endpoint().address())) {
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
                } while (conn_map_.contains(key) && count < 3);

                if (count >= 3) {
                    socket.close();
                    spdlog::warn("Fail To Distribute Connection Key: {}", addr.to_string());
                    continue;
                }

                const auto conn = std::make_shared<UConnection>(std::move(socket), scene);
                spdlog::info("Accept Connection From: {}", addr.to_string());

                conn->SetKey(key);

                server_->SetConnectionCodec(conn);
                server_->SetConnectionHandler(conn);

                conn->ConnectToClient();

                conn_map_[key] = conn;

                if (conn_map_.size() >= 1'000'000'000) {
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

void UGameWorld::RemoveConnection(const std::string_view key) {
    if (!running_)
        return;

    if (std::this_thread::get_id() != tid_) {
        co_spawn(ctx_, [this, key = std::string(key)]() mutable -> awaitable<void> {
            conn_map_.erase(key);
            co_return;
        }, detached);
        return;
    }
#if defined(_WIN32) || defined(_WIN64)
    conn_map_.erase(key);
#else
    conn_map_.erase(std::string(key));
#endif
}

asio::io_context &UGameWorld::GetIOContext() {
    return ctx_;
}

AThreadID UGameWorld::GetThreadID() const {
    return tid_;
}

bool UGameWorld::IsMainThread() const {
    return tid_ == std::this_thread::get_id();
}
