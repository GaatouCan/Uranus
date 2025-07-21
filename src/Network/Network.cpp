#include "Network.h"
#include "Connection.h"
#include "Server.h"
#include "Recycler.h"
#include "Gateway/Gateway.h"
#include "Login/LoginAuth.h"
#include "Config/Config.h"
#include "Internal/Packet.h"

#include <spdlog/spdlog.h>


UNetwork::UNetwork()
    : acceptor_(ctx_),
      pool_(nullptr) {
}

UNetwork::~UNetwork() {
    Stop();

    if (thread_.joinable()) {
        thread_.join();
    }
}

void UNetwork::Initial() {
    if (mState != EModuleState::CREATED)
        return;

    pool_ = GetServer()->CreatePackagePool(ctx_);
    pool_->Initial();

    thread_ = std::thread([this] {
        SPDLOG_INFO("Network Work In Thread[{}]", utils::ThreadIDToInt(std::this_thread::get_id()));

        asio::signal_set signals(ctx_, SIGINT, SIGTERM);
        signals.async_wait([this](auto, auto) {
            Stop();
        });
        ctx_.run();
    });

    mState = EModuleState::INITIALIZED;
}

void UNetwork::Start() {
    if (mState != EModuleState::INITIALIZED)
        return;

    uint16_t port = 8080;
    if (const auto *config = GetServer()->GetModule<UConfig>(); config != nullptr) {
        port = config->GetServerConfig()["server"]["port"].as<uint16_t>();
    }

    co_spawn(ctx_, WaitForClient(port), detached);
    mState = EModuleState::RUNNING;
}

void UNetwork::Stop() {
    if (mState == EModuleState::STOPPED)
        return;

    mState = EModuleState::STOPPED;

    if (!ctx_.stopped()) {
        ctx_.stop();
    }

    connectionMap_.clear();
}


awaitable<void> UNetwork::WaitForClient(uint16_t port) {
    try {
        acceptor_.open(asio::ip::tcp::v4());
        acceptor_.bind({asio::ip::tcp::v4(), port});
        acceptor_.listen(port);

        SPDLOG_INFO("Waiting For Client To Connect - Server Port: {}", port);

        while (mState == EModuleState::RUNNING) {
            auto [ec, socket] = co_await acceptor_.async_accept();

            if (ec) {
                SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, ec.message());
                continue;
            }

            if (socket.is_open()) {
                if (auto *login = GetServer()->GetModule<ULoginAuth>(); login != nullptr) {
                    if (!login->VerifyAddress(socket.remote_endpoint())) {
                        SPDLOG_WARN("Reject Client From {}", socket.remote_endpoint().address().to_string());
                        socket.close();
                        continue;
                    }
                }

                const auto conn = make_shared<UConnection>(this, std::move(socket));

                if (const auto id = conn->GetConnectionID(); id > 0) {
                    std::unique_lock lock(mutex_);
                    connectionMap_[id] = conn;
                } else {
                    SPDLOG_WARN("{:<20} - Failed To Get Connection ID From {}",
                        __FUNCTION__, conn->RemoteAddress().to_string());
                    conn->Disconnect();
                    continue;
                }

                SPDLOG_INFO("{:<20} - New Connection From {} - fd[{}]",
                    __FUNCTION__, conn->RemoteAddress().to_string(), conn->GetConnectionID());

                GetServer()->InitConnection(conn);

                conn->ConnectToClient();
            }
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{} - {}", __FUNCTION__, e.what());
    }
}

io_context &UNetwork::GetIOContext() {
    return ctx_;
}

shared_ptr<UConnection> UNetwork::FindConnection(const int64_t cid) const {
    if (mState != EModuleState::RUNNING)
        return nullptr;

    std::shared_lock lock(mutex_);
    const auto it = connectionMap_.find(cid);
    return it != connectionMap_.end() ? it->second : nullptr;
}

void UNetwork::RemoveConnection(const int64_t cid, const int64_t pid) {
    if (mState != EModuleState::RUNNING)
        return;

    {
        std::unique_lock lock(mutex_);
        connectionMap_.erase(cid);
    }

    if (pid > 0) {
        if (auto *gateway = GetServer()->GetModule<UGateway>()) {
            gateway->OnPlayerLogout(pid);
        }
    }
}

void UNetwork::OnLoginSuccess(const int64_t cid, const int64_t pid, const shared_ptr<IPackageInterface> &pkg) const {
    if (mState != EModuleState::RUNNING)
        return;

    shared_ptr<UConnection> conn;
    {
        std::unique_lock lock(mutex_);
        if (const auto it = connectionMap_.find(cid); it != connectionMap_.end()) {
            conn = it->second;
        }
    }

    if (conn == nullptr) {
        return;
    }

    conn->SetPlayerID(pid);
    conn->SendPackage(pkg);
}

void UNetwork::OnLoginFailure(const int64_t cid, const shared_ptr<IPackageInterface> &pkg) {
    if (mState != EModuleState::RUNNING)
        return;

    shared_ptr<UConnection> conn;
    {
        std::unique_lock lock(mutex_);
        if (const auto it = connectionMap_.find(cid); it != connectionMap_.end()) {
            conn = it->second;
        }
    }

    if (conn == nullptr) {
        return;
    }

    conn->SendPackage(pkg);
    conn->Disconnect();
}

void UNetwork::SendToClient(const int64_t cid, const shared_ptr<IPackageInterface> &pkg) const {
    if (mState != EModuleState::RUNNING)
        return;

    std::shared_lock lock(mutex_);
    if (const auto iter = connectionMap_.find(cid); iter != connectionMap_.end()) {
        if (const auto conn = iter->second) {
            conn->SendPackage(pkg);
        }
    }
}

std::shared_ptr<IPackageInterface> UNetwork::BuildPackage() const {
    if (mState != EModuleState::RUNNING)
        return nullptr;

    return std::dynamic_pointer_cast<IPackageInterface>(pool_->Acquire());
}
