#include "Connection.h"
#include "Network.h"
#include "Server.h"
#include "Gateway/Gateway.h"
#include "Login/LoginAuth.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>


using namespace asio::experimental::awaitable_operators;
using namespace std::literals::chrono_literals;


UConnection::UConnection(UNetwork *module, ATcpSocket socket)
    : module_(module),
      socket_(std::move(socket)),
      codec_(nullptr),
      channel_(socket_.get_executor(), 1024),
      watchdog_(socket_.get_executor()),
      expiration_(std::chrono::seconds(30)),
      playerId_(-1) {
    id_ = static_cast<int64_t>(socket_.native_handle());
    socket_.set_option(asio::ip::tcp::socket::keep_alive(true));
}

UConnection::~UConnection() {
    Disconnect();
}

ATcpSocket &UConnection::GetSocket() {
    return socket_;
}

TConcurrentChannel<void(std::error_code, shared_ptr<IPackageInterface>)> &UConnection::GetChannel() {
    return channel_;
}

void UConnection::SetExpireSecond(const int sec) {
    expiration_ = std::chrono::seconds(sec);
}

void UConnection::SetPlayerID(const int64_t id) {
    if (playerId_ > 0)
        return;

    playerId_ = id;
}

void UConnection::ConnectToClient() {
    assert(codec_ != nullptr);

    receiveTime_ = std::chrono::steady_clock::now();

    co_spawn(socket_.get_executor(), [self = shared_from_this()]() -> awaitable<void> {
        co_await (
            self->ReadPackage() ||
            self->WritePackage() ||
            self->Watchdog()
        );
    }, detached);
}

void UConnection::Disconnect() {
    if (!socket_.is_open())
        return;

    socket_.close();
    watchdog_.cancel();
    socket_.close();

    module_->RemoveConnection(id_, playerId_);

    if (playerId_ > 0) {
        playerId_ = -1;
    }
}

UNetwork *UConnection::GetNetworkModule() const {
    return module_;
}

UServer *UConnection::GetServer() const {
    return module_->GetServer();
}

std::shared_ptr<IPackageInterface> UConnection::BuildPackage() const {
    return module_->BuildPackage();
}

asio::ip::address UConnection::RemoteAddress() const {
    if (socket_.is_open()) {
        return socket_.remote_endpoint().address();
    }
    return {};
}

int64_t UConnection::GetConnectionID() const {
    return id_;
}

int64_t UConnection::GetPlayerID() const {
    return playerId_;
}

void UConnection::SendPackage(const std::shared_ptr<IPackageInterface> &pkg) {
    if (pkg == nullptr)
        return;

    co_spawn(socket_.get_executor(), [self = shared_from_this(), pkg]() -> awaitable<void> {
        co_await self->channel_.async_send(std::error_code{}, pkg);
    }, detached);
}

awaitable<void> UConnection::WritePackage() {
    try {
        while (socket_.is_open()) {
            const auto [ec, pkg] = co_await channel_.async_receive();
            if (ec || pkg == nullptr)
                co_return;

            if (const auto ret = co_await codec_->Encode(pkg); !ret) {
                SPDLOG_WARN("{:<20} - Failed To Write Package", __FUNCTION__);
                Disconnect();
                break;
            }

            // Can Do Something Here
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
        Disconnect();
    }
}

awaitable<void> UConnection::ReadPackage() {
    try {
        while (socket_.is_open()) {
            const auto pkg = BuildPackage();
            if (pkg == nullptr)
                co_return;

            if (const auto ret = co_await codec_->Decode(pkg); !ret) {
                SPDLOG_WARN("{:<20} - Failed To Read Package", __FUNCTION__);
                Disconnect();
                break;
            }

            const auto now = std::chrono::steady_clock::now();

            // Run The Login Branch
            if (playerId_ < 0) {
                // Do Not Try Login Too Frequently
                if (now - receiveTime_ > std::chrono::seconds(3)) {
                    --playerId_;

                    // Try Login Failed 3 Times Then Disconnect This
                    if (playerId_ < -3) {
                        SPDLOG_WARN("{:<20} - Connection[{}] Try Login Too Many Times", __FUNCTION__, RemoteAddress().to_string());
                        Disconnect();
                        break;
                    }

                    // Handle Login Logic
                    if (auto *login = GetServer()->GetModule<ULoginAuth>(); login != nullptr) {
                        login->OnPlayerLogin(id_, pkg);
                    }
                }

                receiveTime_ = now;
                continue;
            }

            // Update Receive Time Point For Watchdog
            receiveTime_ = now;

            if (const auto *gateway = GetServer()->GetModule<UGateway>(); gateway != nullptr) {
                if (pkg->GetPackageID() == 1001) {
                    gateway->OnHeartBeat(playerId_, pkg);
                } else {
                    gateway->OnClientPackage(playerId_, pkg);
                }
            }
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
        Disconnect();
    }
}

awaitable<void> UConnection::Watchdog() {
    if (expiration_ == ASteadyDuration::zero()) {
        co_return;
    }

    try {
        ASteadyTimePoint now;

        do {
            watchdog_.expires_at(receiveTime_ + expiration_);

            if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                SPDLOG_DEBUG("{:<20} - Timer Canceled.", __FUNCTION__);
                co_return;
            }

            now = std::chrono::steady_clock::now();
        } while ((receiveTime_ + expiration_) > now);

        if (socket_.is_open()) {
            SPDLOG_WARN("{:<20} - Watchdog Timeout", __FUNCTION__);
            Disconnect();
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
    }
}
