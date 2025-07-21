#include "Server.h"
#include "Config/Config.h"
#include "Internal/PacketCodec.h"
#include "Recycler.h"
#include "Network/Connection.h"
#include "Utils.h"

#include <spdlog/spdlog.h>
#include <mimalloc-new-delete.h>


UServer::UServer()
    : guard_(asio::make_work_guard(context_)),
      bInitialized(false),
      bRunning(false),
      bShutdown(false) {
}

UServer::~UServer() {
    for (auto &th: workerList_) {
        if (th.joinable()) {
            th.join();
        }
    }

    // for (const auto &module: moduleMap_ | std::views::values) {
    //     delete module;
    // }
}

asio::io_context &UServer::GetIOContext() {
    return context_;
}

IModuleBase *UServer::GetModule(const std::string &name) const {
    const auto iter = nameToType_.find(name);
    if (iter == nameToType_.end()) {
        return nullptr;
    }

    const auto module_iter = moduleMap_.find(iter->second);
    return module_iter != moduleMap_.end() ? module_iter->second.get() : nullptr;
}

IServerHandler *UServer::GetServerHandler() const {
    return handler_.get();
}

std::shared_ptr<IRecyclerBase> UServer::CreatePackagePool(io_context &ctx) const {
    if (handler_ != nullptr && !handler_->IsUseCustomPackage()) {
        return handler_->CreatePackagePool(ctx);
    }

    return std::make_shared<TRecycler<FPacket>>(ctx);
}

void UServer::InitLoginAuth(ULoginAuth *auth) const {
    if (handler_ != nullptr)
        handler_->InitLoginAuth(auth);
}

void UServer::InitConnection(const std::shared_ptr<UConnection> &conn) const {
    if (handler_ != nullptr && !handler_->IsUseCustomPackage()) {
        handler_->InitConnection(conn);
        return;
    }

    conn->SetPackageCodec<UPacketCodec>();
}

void UServer::Initial() {
    if (bInitialized)
        return;

    SPDLOG_INFO("Loading Service From {}", CORE_SERVICE_DIRECTORY);

    if (!std::filesystem::exists(PLAYER_AGENT_DIRECTORY)) {
        try {
            std::filesystem::create_directory(PLAYER_AGENT_DIRECTORY);
        } catch (const std::exception &e) {
            SPDLOG_CRITICAL("{:<20} - Failed To Create Player Agent Directory {}", __FUNCTION__, e.what());
            Shutdown();
            exit(-1);
        }
    }

    if (!std::filesystem::exists(CORE_SERVICE_DIRECTORY)) {
        try {
            std::filesystem::create_directory(CORE_SERVICE_DIRECTORY);
        } catch (const std::exception &e) {
            SPDLOG_CRITICAL("{:<20} - Failed To Create Service Directory {}", __FUNCTION__, e.what());
            Shutdown();
            exit(-2);
        }
    }

    if (!std::filesystem::exists(EXTEND_SERVICE_DIRECTORY)) {
        try {
            std::filesystem::create_directory(EXTEND_SERVICE_DIRECTORY);
        } catch (const std::exception &e) {
            SPDLOG_CRITICAL("{:<20} - Failed To Create Extend Directory {}", __FUNCTION__, e.what());
            Shutdown();
            exit(-3);
        }
    }

    SPDLOG_INFO("Initializing Modules...");
    for (const auto &type : moduleOrdered_) {
        if (auto *module = moduleMap_[type].get()) {
            module->Initial();
            SPDLOG_INFO("{} Initialized.", module->GetModuleName());
        }
    }
    SPDLOG_INFO("Modules Initialization Completed!");

    const auto *config = GetModule<UConfig>();
    if (config == nullptr) {
        SPDLOG_CRITICAL("Fail To Found Config Module!");
        Shutdown();
        exit(-4);
    }

    const int count = config->GetServerConfig()["server"]["worker"].as<int>();
    for (int idx = 1; idx < count; ++idx) {
        workerList_.emplace_back([this, idx] {
            const int64_t tid = utils::ThreadIDToInt(std::this_thread::get_id());

            SPDLOG_INFO("Worker[{}] Is Running In Thread: {}", idx, tid);
            context_.run();
        });
    }
    SPDLOG_INFO("Server Run With {} Worker(s)", count);
    SPDLOG_INFO("Server Initialization Completed!");

    bInitialized = true;
}

void UServer::Run() {
    if (bRunning)
        return;

    for (const auto &type : moduleOrdered_) {
        if (auto *module = moduleMap_[type].get()) {
            module->Start();
            SPDLOG_INFO("{} Started.", module->GetModuleName());
        }
    }

    asio::signal_set signals(context_, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        Shutdown();
    });

    bRunning = true;
    SPDLOG_INFO("Server Is Running...");

    context_.run();
}

void UServer::Shutdown() {
    if (bShutdown)
        return;

    bShutdown = true;
    SPDLOG_INFO("Shutting Down...");

    for (auto iter = moduleOrdered_.rbegin(); iter != moduleOrdered_.rend(); ++iter) {
        if (auto *module = moduleMap_[*iter].get()) {
            module->Stop();
            SPDLOG_INFO("{} Stopped.", module->GetModuleName());
        }
    }

    guard_.reset();
    context_.stop();

    SPDLOG_INFO("Server Shutdown Completed!");
}

bool UServer::IsInitialized() const {
    return bInitialized && !bShutdown;
}

bool UServer::IsRunning() const {
    return bRunning && !bShutdown;
}

bool UServer::IsShutdown() const {
    return bShutdown;
}
