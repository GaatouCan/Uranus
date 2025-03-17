#include "../include/game_world.h"
#include "../include/server_logic.h"
#include "../include/connection.h"

#include "../include/scene/scene_manager.h"
#include "../include/scene/main_scene.h"
#include "../include/reactor/global_queue.h"
#include "../include/config_manager.h"
#include "../include/login_authenticator.h"
#include "../include/proto_route.h"

#include "../include/system/database/database_system.h"
#include "../include/system/event/event_system.h"
#include "../include/system/timer/timer_system.h"
#include "../include/system/plugin/plugin_system.h"
#include "../include/system/manager/manager_system.h"
#include "../include/system/command/command_system.h"

#include <ranges>
#include <random>


UGameWorld::UGameWorld()
    : acceptor_(context_),
      module_(nullptr),
      server_(nullptr),
      destroyer_(nullptr),
      fullTimer_(context_),
      inited_(false),
      running_(false) {

    configManager_ = new UConfigManager();
    sceneManager_ = new USceneManager(this);
    globalQueue_ = new UGlobalQueue(this);
    loginAuthenticator_ = new ULoginAuthenticator(this);
    protoRoute_ = new UProtoRoute(this);

    // Create Sub System
    createSystem<UDatabaseSystem>(2);
    createSystem<UTimerSystem>(3);
    createSystem<UCommandSystem>(4);
    createSystem<UManagerSystem>(9);
    createSystem<UEventSystem>(10);
    createSystem<UPluginSystem>(11);
}

UGameWorld::~UGameWorld() {
    shutdown();

    while (!destroyPriority_.empty()) {
        auto [priority, type] = destroyPriority_.top();
        destroyPriority_.pop();

        const auto iter = systemMap_.find(type);
        if (iter == systemMap_.end())
            continue;

        spdlog::info("{} Destroyed.", iter->second->getSystemName());
        delete iter->second;

        systemMap_.erase(iter);
    }

    // 正常情况下map应该是空了 但以防万一还是再遍历一次
    for (const auto sys: systemMap_ | std::views::values) {
        delete sys;
    }

    delete protoRoute_;
    delete globalQueue_;
    delete sceneManager_;
    delete loginAuthenticator_;
    delete configManager_;

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

UGameWorld &UGameWorld::init(const std::string &path) {
    if (!loadServerDLL(path)) {
        shutdown();
        exit(-1);
    }

    tid_ = std::this_thread::get_id();

    configManager_->init();
    assert(configManager_->loaded());

    sceneManager_->init();
    globalQueue_->init();
    loginAuthenticator_->init();

    while (!initPriority_.empty()) {
        auto [priority, type] = initPriority_.top();
        initPriority_.pop();

        const auto iter = systemMap_.find(type);
        if (iter == systemMap_.end())
            continue;

        iter->second->init();
        spdlog::info("{} Initialized.", iter->second->getSystemName());
    }

    inited_ = true;

    return *this;
}

UGameWorld &UGameWorld::run() {
    running_ = true;

    asio::signal_set signals(context_, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        shutdown();
    });

    co_spawn(context_, waitForConnect(), detached);
    context_.run();

    return *this;
}

UGameWorld &UGameWorld::shutdown() {
    if (!inited_)
        return *this;

    if (!running_)
        return *this;

    spdlog::info("Server Shutting Down...");
    running_ = false;

    if (!context_.stopped())
        context_.stop();

    connectionMap_.clear();

    return *this;
}

void UGameWorld::forceDisconnectAll() {
    if (!running_)
        return;

    if (std::this_thread::get_id() != tid_) {
        co_spawn(context_, [this]() mutable -> awaitable<void> {
            for (const auto &conn : connectionMap_ | std::views::values) {
                conn->disconnect();
            }
            connectionMap_.clear();
            co_return;
        }, detached);
        return;
    }

    for (const auto &conn : connectionMap_ | std::views::values) {
        conn->disconnect();
    }
    connectionMap_.clear();
}

void UGameWorld::removeConnection(const std::string &key) {
    if (!running_)
        return;

    if (std::this_thread::get_id() != tid_) {
        co_spawn(context_, [this, key]() mutable -> awaitable<void> {
            connectionMap_.erase(key);
            co_return;
        }, detached);
        return;
    }
    connectionMap_.erase(key);
}

UConfigManager *UGameWorld::getConfigManager() const {
    return configManager_;
}

USceneManager *UGameWorld::getSceneManager() const {
    return sceneManager_;
}

ULoginAuthenticator * UGameWorld::getLoginAuthenticator() const {
    return loginAuthenticator_;
}

UProtoRoute * UGameWorld::getProtoRoute() const {
    return protoRoute_;
}

UGlobalQueue* UGameWorld::getGlobalQueue() const
{
    return globalQueue_;
}

ISubSystem *UGameWorld::getSystemByName(const std::string_view sys) const {
    if (const auto it = nameToSystem_.find(sys); it != nameToSystem_.end()) {
        return it->second;
    }
    return nullptr;
}

const YAML::Node &UGameWorld::getServerConfig() {
    if (!configManager_->loaded()) {
        spdlog::critical("{} - ConfigSystem not loaded", __FUNCTION__);
        shutdown();
        exit(-1);
    }

    return configManager_->getServerConfig();
}

int32_t UGameWorld::getServerID() {
    const auto &cfg = getServerConfig();
    return cfg["server"]["cross_id"].as<int32_t>();
}

IServerLogic * UGameWorld::getServerLogic() const {
    return server_;
}

bool UGameWorld::loadServerDLL(const std::string &path) {
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

    const auto creator = reinterpret_cast<AServerCreator>(dlsym(module_, "CreateServer"));
    const auto destroyer = reinterpret_cast<AServerDestroyer>(dlsym(module_, "DestroyServer"));

    if (creator == nullptr || destroyer == nullptr) {
        spdlog::error("Failed to load DLL function: {}", path);
        dlclose(module_);
        return false;
    }
#endif

    server_ = creator(this);
    destroyer_ = destroyer;

    server_->initGameWorld();

    configManager_->abort();
    protoRoute_->abort();
    loginAuthenticator_->abort();

    spdlog::info("Loaded Dynamic-link Library {} Success.", path);

    return true;
}

awaitable<void> UGameWorld::waitForConnect() {
    const auto &config = getServerConfig();

    try {
        acceptor_.open(tcp::v4());
        acceptor_.bind({tcp::v4(), config["server"]["port"].as<uint16_t>()});
        acceptor_.listen();

        spdlog::info("Waiting For Client To Connect - Server Port: {}", config["server"]["port"].as<uint16_t>());

        while (running_) {
            const auto scene = dynamic_cast<UMainScene *>(sceneManager_->getNextMainScene());
            if (scene == nullptr) {
                spdlog::critical("{} - Failed to get main scene.", __FUNCTION__);
                shutdown();
                exit(-1);
            }

            auto [ec, socket] = co_await acceptor_.async_accept(scene->getIOContext());

            if (ec) {
                spdlog::error("{} - {}", __FUNCTION__, ec.message());
                continue;
            }

            if (socket.is_open()) {
                const auto addr = socket.remote_endpoint().address();
                spdlog::info("New Connection From: {}", addr.to_string());

                if (!loginAuthenticator_->verifyAddress(socket.remote_endpoint().address())) {
                    socket.close();
                    spdlog::warn("Rejected Connection From: {}", addr.to_string());
                    continue;
                }

                std::string key;
                int count = 0;

                static std::random_device randomDevice;
                static std::mt19937 generator(randomDevice());
                static std::uniform_int_distribution distribution(100, 999);

                do {
                    key = fmt::format("{}-{}-{}", addr.to_string(), utils::UnixTime(), distribution(generator));
                    count++;
                } while (connectionMap_.contains(key) && count < 3);

                if (count >= 3) {
                    socket.close();
                    spdlog::warn("Fail To Distribute Connection Key: {}", addr.to_string());
                    continue;
                }

                const auto conn = std::make_shared<UConnection>(std::move(socket), scene);
                spdlog::info("Accept Connection From: {}", addr.to_string());

                conn->setKey(key);

                server_->setConnectionCodec(conn);
                server_->setConnectionHandler(conn);

                conn->connectToClient();

                connectionMap_[key] = conn;

                if (connectionMap_.size() >= 1'000'000'000) {
                    fullTimer_.expires_after(10s);
                    co_await fullTimer_.async_wait();
                }
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
        shutdown();
    }
}

void UGameWorld::removeConnection(const std::string_view key) {
    if (!running_)
        return;

    if (std::this_thread::get_id() != tid_) {
        co_spawn(context_, [this, key = std::string(key)]() mutable -> awaitable<void> {
            connectionMap_.erase(key);
            co_return;
        }, detached);
        return;
    }
#if defined(_WIN32) || defined(_WIN64)
    connectionMap_.erase(key);
#else
    conn_map_.erase(std::string(key));
#endif
}

asio::io_context &UGameWorld::getIOContext() {
    return context_;
}

AThreadID UGameWorld::getThreadID() const {
    return tid_;
}

bool UGameWorld::isMainThread() const {
    return tid_ == std::this_thread::get_id();
}
