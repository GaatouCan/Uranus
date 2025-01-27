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
    : mAcceptor(mContext),
      mModule(nullptr),
      mServer(nullptr),
      mServerDestroyer(nullptr),
      mFullTimer(mContext),
      bInited(false),
      bRunning(false) {

    mConfigManager = new UConfigManager();
    mSceneManager = new USceneManager(this);
    mGlobalQueue = new UGlobalQueue(this);
    mLoginAuthenticator = new ULoginAuthenticator(this);
    mProtocolRoute = new UProtocolRoute(this);

    // Create Sub System
    // CreateSystem<UCommandSystem>(2);
    CreateSystem<UTimerSystem>(3);
    CreateSystem<UManagerSystem>(4);
    CreateSystem<UEventSystem>(5);
    CreateSystem<UPluginSystem>(6);
}

UGameWorld::~UGameWorld() {
    Shutdown();

    while (!mDestPriority.empty()) {
        auto [priority, type] = mDestPriority.top();
        mDestPriority.pop();

        const auto iter = mSystemMap.find(type);
        if (iter == mSystemMap.end())
            continue;

        spdlog::info("{} Destroyed.", iter->second->GetSystemName());
        delete iter->second;

        mSystemMap.erase(iter);
    }

    // 正常情况下map应该是空了 但以防万一还是再遍历一次
    for (const auto sys: mSystemMap | std::views::values) {
        delete sys;
    }

    delete mProtocolRoute;
    delete mGlobalQueue;
    delete mSceneManager;
    delete mLoginAuthenticator;
    delete mConfigManager;

    if (mServer && mServerDestroyer) {
        mServerDestroyer(mServer);
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(mModule);
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

    mThreadID = std::this_thread::get_id();

    mConfigManager->Init();
    assert(mConfigManager->IsLoaded());

    const auto &config = GetServerConfig();

    FPackage::LoadConfig(config);
    UPackagePool::LoadConfig(config);

    UPackagePool::SetPackageBuilder(&FPackage::CreatePackage);
    UPackagePool::SetPackageInitializer(&FPackage::InitPackage);

    mSceneManager->Init();
    mGlobalQueue->Init();
    mLoginAuthenticator->Init();

    while (!mInitPriority.empty()) {
        auto [priority, type] = mInitPriority.top();
        mInitPriority.pop();

        const auto iter = mSystemMap.find(type);
        if (iter == mSystemMap.end())
            continue;

        iter->second->Init();
        spdlog::info("{} Initialized.", iter->second->GetSystemName());
    }

    bInited = true;

    return *this;
}

UGameWorld &UGameWorld::Run() {
    bRunning = true;

    asio::signal_set signals(mContext, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        Shutdown();
    });

    co_spawn(mContext, WaitForConnect(), detached);
    mContext.run();

    return *this;
}

UGameWorld &UGameWorld::Shutdown() {
    if (!bInited)
        return *this;

    if (!bRunning)
        return *this;

    spdlog::info("Server Shutting Down...");
    bRunning = false;

    if (!mContext.stopped())
        mContext.stop();

    // for (const auto& conn : mConnectionMap | std::views::values)
    //     conn->Disconnect();

    mConnectionMap.clear();

    return *this;
}

// void UGameWorld::FilterConnection(const std::function<void(const AConnectionPointer &)> &filter) {
//     mConnectionFilter = filter;
// }

void UGameWorld::RemoveConnection(const std::string &key) {
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

UConfigManager *UGameWorld::GetConfigManager() const {
    return mConfigManager;
}

USceneManager *UGameWorld::GetSceneManager() const {
    return mSceneManager;
}

ULoginAuthenticator * UGameWorld::GetLoginAuthenticator() const {
    return mLoginAuthenticator;
}

UProtocolRoute * UGameWorld::GetProtocolRoute() const {
    return mProtocolRoute;
}

UGlobalQueue* UGameWorld::GetGlobalQueue() const
{
    return mGlobalQueue;
}

ISubSystem *UGameWorld::GetSystemByName(const std::string_view sys) const {
    if (const auto it = mNameToSystem.find(sys); it != mNameToSystem.end()) {
        return it->second;
    }
    return nullptr;
}

const YAML::Node &UGameWorld::GetServerConfig() {
    if (!mConfigManager->IsLoaded()) {
        spdlog::critical("{} - ConfigSystem not loaded", __FUNCTION__);
        Shutdown();
        exit(-1);
    }

    return mConfigManager->GetServerConfig();
}

uint32_t UGameWorld::GetServerID() {
    const auto &cfg = GetServerConfig();
    return cfg["server"]["cross_id"].as<uint32_t>();
}

bool UGameWorld::LoadServerDLL(const std::string &path) {
#if defined(_WIN32) || defined(_WIN64)
    mModule = LoadLibrary(path.c_str());
#else
    mModule = dlopen(path.c_str(), RTLD_LAZY);
#endif

    if (mModule == nullptr) {
        spdlog::error("Failed to load server dll: {}", path);
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)
    const auto creator = reinterpret_cast<AServerCreator>(GetProcAddress(mModule, "CreateServer"));
    const auto destroyer = reinterpret_cast<AServerDestroyer>(GetProcAddress(mModule, "DestroyServer"));
#else
    const auto creator = reinterpret_cast<AServerCreator>(dlsym(mModule, "CreateServer"));
    const auto destroyer = reinterpret_cast<AServerDestroyer>(dlsym(mModule, "DestroyServer"));
#endif

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("Failed to load DLL function: {}", path);

#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(mModule);
#else
        dlclose(mModule);
#endif

        return false;
    }

    mServer = creator(this);
    mServerDestroyer = destroyer;

    mServer->InitGameWorld();

    mConfigManager->Abort();
    mProtocolRoute->AbortHandler();
    mLoginAuthenticator->AbortHandler();

    spdlog::info("Loaded DLL {} Success.", path);

    return true;
}

awaitable<void> UGameWorld::WaitForConnect() {
    const auto &config = GetServerConfig();

    try {
        mAcceptor.open(tcp::v4());
        mAcceptor.bind({tcp::v4(), config["server"]["port"].as<uint16_t>()});
        mAcceptor.listen();

        spdlog::info("Waiting For Client To Connect - Server Port: {}", config["server"]["port"].as<uint16_t>());

        while (bRunning) {
            // PackagePool and io_context in per sub thread
            // auto &[pool, ctx, tid, index] = mContextPool->NextContextNode();
            const auto scene = dynamic_cast<UMainScene *>(mSceneManager->GetNextMainScene());
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

                static std::random_device sRandomDevice;
                static std::mt19937 sGenerator(sRandomDevice());
                static std::uniform_int_distribution<> sDistribution(100, 999);

                do {
                    key = fmt::format("{}-{}-{}", addr.to_string(), utils::UnixTime(), sDistribution(sGenerator));
                    count++;
                } while (mConnectionMap.contains(key) && count < 3);

                if (count >= 3) {
                    socket.close();
                    spdlog::warn("Fail To Distribute Connection Key: {}", addr.to_string());
                    continue;
                }

                const auto conn = std::make_shared<UConnection>(std::move(socket), scene);
                spdlog::info("Accept Connection From: {}", addr.to_string());

                conn->SetKey(key);

                // if (mConnectionFilter)
                //     std::invoke(mConnectionFilter, conn);
                //
                // if (!conn->HasCodecSet())
                conn->SetPackageCodec<UPackageCodecImpl>();
                mServer->SetConnectionHandler(conn);

                // if (!conn->HasHandlerSet()) {
                //     spdlog::warn("Connection handler is not set - [{}]", key);
                // }

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

void UGameWorld::RemoveConnection(const std::string_view key) {
    if (std::this_thread::get_id() != mThreadID) {
        co_spawn(mContext, [this, key]() mutable -> awaitable<void> {
            mConnectionMap.erase(key);
            co_return;
        }, detached);
        return;
    }
    mConnectionMap.erase(key);
}

asio::io_context &UGameWorld::GetIOContext() {
    return mContext;
}

AThreadID UGameWorld::GetThreadID() const {
    return mThreadID;
}

bool UGameWorld::IsMainThread() const {
    return mThreadID == std::this_thread::get_id();
}
