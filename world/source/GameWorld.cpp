#include "../include/GameWorld.h"
#include "../include/ServerLogic.h"
#include "../include/impl/Package.h"
#include "../include/Connection.h"
#include "../include/PackagePool.h"
#include "../include/impl/PackageCodecImpl.h"

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


UGameWorld::UGameWorld()
    : acceptor(ctx_),
      module_(nullptr),
      server_(nullptr),
      serverDestroyer_(nullptr),
      fullTimer_(ctx_),
      inited_(false),
      running_(false) {

    configManager_ = new UConfigManager();
    sceneManager_ = new USceneManager(this);
    globalQueue_ = new UGlobalQueue(this);
    loginAuthenticator_ = new ULoginAuthenticator(this);
    protocolRoute_ = new UProtocolRoute(this);

    // Create Sub System
    CreateSystem<UTimerSystem>(3);
    CreateSystem<UManagerSystem>(4);
    CreateSystem<UEventSystem>(5);
    CreateSystem<UPluginSystem>(6);
}

UGameWorld::~UGameWorld() {
    Shutdown();

    while (!destPriority_.empty()) {
        auto [priority, type] = destPriority_.top();
        destPriority_.pop();

        const auto iter = systemMap_.find(type);
        if (iter == systemMap_.end())
            continue;

        spdlog::info("{} Destroyed.", iter->second->GetSystemName());
        delete iter->second;

        systemMap_.erase(iter);
    }

    // 正常情况下map应该是空了 但以防万一还是再遍历一次
    for (const auto sys: systemMap_ | std::views::values) {
        delete sys;
    }

    delete protocolRoute_;
    delete globalQueue_;
    delete sceneManager_;
    delete loginAuthenticator_;
    delete configManager_;

    if (server_ && serverDestroyer_) {
        serverDestroyer_(server_);
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

UGameWorld &UGameWorld::Init(const std::string &dllPath) {
    if (!LoadServerDLL(dllPath)) {
        Shutdown();
        exit(-1);
    }

    worldThreadId_ = std::this_thread::get_id();

    configManager_->Init();
    assert(configManager_->IsLoaded());

    const auto &config = GetServerConfig();

    FPackage::LoadConfig(config);
    UPackagePool::LoadConfig(config);

    UPackagePool::SetPackageBuilder(&FPackage::CreatePackage);
    UPackagePool::SetPackageInitializer(&FPackage::InitPackage);

    sceneManager_->Init();
    globalQueue_->Init();
    loginAuthenticator_->Init();

    while (!initPriority_.empty()) {
        auto [priority, type] = initPriority_.top();
        initPriority_.pop();

        const auto iter = systemMap_.find(type);
        if (iter == systemMap_.end())
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

    connectionMap_.clear();

    return *this;
}

void UGameWorld::RemoveConnection(const std::string &key) {
    if (!running_)
        return;

    if (std::this_thread::get_id() != worldThreadId_) {
        co_spawn(ctx_, [this, key]() mutable -> awaitable<void> {
            connectionMap_.erase(key);
            co_return;
        }, detached);
        return;
    }
    connectionMap_.erase(key);
}

UConfigManager *UGameWorld::GetConfigManager() const {
    return configManager_;
}

USceneManager *UGameWorld::GetSceneManager() const {
    return sceneManager_;
}

ULoginAuthenticator * UGameWorld::GetLoginAuthenticator() const {
    return loginAuthenticator_;
}

UProtocolRoute * UGameWorld::GetProtocolRoute() const {
    return protocolRoute_;
}

UGlobalQueue* UGameWorld::GetGlobalQueue() const
{
    return globalQueue_;
}

ISubSystem *UGameWorld::GetSystemByName(const std::string_view sys) const {
    if (const auto it = nameToSystem_.find(sys); it != nameToSystem_.end()) {
        return it->second;
    }
    return nullptr;
}

const YAML::Node &UGameWorld::GetServerConfig() {
    if (!configManager_->IsLoaded()) {
        spdlog::critical("{} - ConfigSystem not loaded", __FUNCTION__);
        Shutdown();
        exit(-1);
    }

    return configManager_->GetServerConfig();
}

uint32_t UGameWorld::GetServerID() {
    const auto &cfg = GetServerConfig();
    return cfg["server"]["cross_id"].as<uint32_t>();
}

bool UGameWorld::LoadServerDLL(const std::string &path) {
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
    const auto creator = reinterpret_cast<AServerCreator>(GetProcAddress(module_, "CreateServer"));
    const auto destroyer = reinterpret_cast<AServerDestroyer>(GetProcAddress(module_, "DestroyServer"));
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
    serverDestroyer_ = destroyer;

    server_->InitGameWorld();

    configManager_->Abort();
    protocolRoute_->AbortHandler();
    loginAuthenticator_->AbortHandler();

    spdlog::info("Loaded DLL {} Success.", path);

    return true;
}

awaitable<void> UGameWorld::WaitForConnect() {
    const auto &config = GetServerConfig();

    try {
        acceptor.open(tcp::v4());
        acceptor.bind({tcp::v4(), config["server"]["port"].as<uint16_t>()});
        acceptor.listen();

        spdlog::info("Waiting For Client To Connect - Server Port: {}", config["server"]["port"].as<uint16_t>());

        while (running_) {
            const auto scene = dynamic_cast<UMainScene *>(sceneManager_->GetNextMainScene());
            if (scene == nullptr) {
                spdlog::critical("{} - Failed to get main scene.", __FUNCTION__);
                Shutdown();
                exit(-1);
            }

            if (auto socket = co_await acceptor.async_accept(scene->GetIOContext()); socket.is_open()) {
                const auto addr = socket.remote_endpoint().address();
                spdlog::info("New Connection From: {}", addr.to_string());

                if (!loginAuthenticator_->VerifyAddress(socket.remote_endpoint().address())) {
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
                } while (connectionMap_.contains(key) && count < 3);

                if (count >= 3) {
                    socket.close();
                    spdlog::warn("Fail To Distribute Connection Key: {}", addr.to_string());
                    continue;
                }

                const auto conn = std::make_shared<UConnection>(std::move(socket), scene);
                spdlog::info("Accept Connection From: {}", addr.to_string());

                conn->SetKey(key);
                conn->SetPackageCodec<UPackageCodecImpl>();

                server_->SetConnectionHandler(conn);

                conn->ConnectToClient();

                connectionMap_[key] = conn;

                if (connectionMap_.size() >= 1'000'000'000) {
                    fullTimer_.expires_after(10s);
                    co_await fullTimer_.async_wait();
                }
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
        Shutdown();
    }
}

void UGameWorld::RemoveConnection(const std::string_view key) {
    if (std::this_thread::get_id() != worldThreadId_) {
        co_spawn(ctx_, [this, key]() mutable -> awaitable<void> {
            connectionMap_.erase(key);
            co_return;
        }, detached);
        return;
    }
    connectionMap_.erase(key);
}

asio::io_context &UGameWorld::GetIOContext() {
    return ctx_;
}

AThreadID UGameWorld::GetThreadID() const {
    return worldThreadId_;
}

bool UGameWorld::IsMainThread() const {
    return worldThreadId_ == std::this_thread::get_id();
}
