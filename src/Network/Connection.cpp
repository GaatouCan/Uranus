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
    : mModule(module),
      mSocket(std::move(socket)),
      mCodec(nullptr),
      mChannel(mSocket.get_executor(), 1024),
      mWatchdog(mSocket.get_executor()),
      mExpiration(std::chrono::seconds(30)),
      mPlayerID(-1) {
    mID = static_cast<int64_t>(mSocket.native_handle());
}

UConnection::~UConnection() {
    Disconnect();
}

ATcpSocket &UConnection::GetSocket() {
    return mSocket;
}

APackageChannel &UConnection::GetChannel() {
    return mChannel;
}

void UConnection::SetExpireSecond(const int sec) {
    mExpiration = std::chrono::seconds(sec);
}

void UConnection::SetPlayerID(const int64_t id) {
    if (mPlayerID > 0)
        return;

    mPlayerID = id;
}

void UConnection::ConnectToClient() {
    assert(mCodec != nullptr);

    mReceiveTime = std::chrono::steady_clock::now();

    co_spawn(mSocket.get_executor(), [self = shared_from_this()]() -> awaitable<void> {
        co_await (
            self->ReadPackage() ||
            self->WritePackage() ||
            self->Watchdog()
        );
    }, detached);
}

void UConnection::Disconnect() {
    if (!mSocket.is_open())
        return;

    mSocket.close();
    mWatchdog.cancel();
    mSocket.close();

    mModule->RemoveConnection(mID, mPlayerID);

    if (mPlayerID > 0) {
        mPlayerID = -1;
    }
}

UNetwork *UConnection::GetNetworkModule() const {
    return mModule;
}

UServer *UConnection::GetServer() const {
    return mModule->GetServer();
}

std::shared_ptr<IPackageBase> UConnection::BuildPackage() const {
    return mModule->BuildPackage();
}

asio::ip::address UConnection::RemoteAddress() const {
    if (mSocket.is_open()) {
        return mSocket.remote_endpoint().address();
    }
    return {};
}

int64_t UConnection::GetConnectionID() const {
    return mID;
}

int64_t UConnection::GetPlayerID() const {
    return mPlayerID;
}

void UConnection::SendPackage(const std::shared_ptr<IPackageBase> &pkg) {
    if (pkg == nullptr)
        return;

    co_spawn(mSocket.get_executor(), [self = shared_from_this(), pkg]() -> awaitable<void> {
        co_await self->mChannel.async_send(std::error_code{}, pkg);
    }, detached);
}

awaitable<void> UConnection::WritePackage() {
    try {
        while (mSocket.is_open()) {
            const auto [ec, pkg] = co_await mChannel.async_receive();
            if (ec || pkg == nullptr)
                co_return;

            if (const auto ret = co_await mCodec->Encode(pkg); !ret) {
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
        while (mSocket.is_open()) {
            const auto pkg = BuildPackage();
            if (pkg == nullptr)
                co_return;

            if (const auto ret = co_await mCodec->Decode(pkg); !ret) {
                SPDLOG_WARN("{:<20} - Failed To Read Package", __FUNCTION__);
                Disconnect();
                break;
            }

            const auto now = std::chrono::steady_clock::now();

            // Run The Login Branch
            if (mPlayerID < 0) {
                // Do Not Try Login Too Frequently
                if (now - mReceiveTime > std::chrono::seconds(3)) {
                    --mPlayerID;

                    // Try Login Failed 3 Times Then Disconnect This
                    if (mPlayerID < -3) {
                        SPDLOG_WARN("{:<20} - Connection[{}] Try Login Too Many Times", __FUNCTION__, RemoteAddress().to_string());
                        Disconnect();
                        break;
                    }

                    // Handle Login Logic
                    if (auto *login = GetServer()->GetModule<ULoginAuth>(); login != nullptr) {
                        login->OnPlayerLogin(mID, pkg);
                    }
                }

                mReceiveTime = now;
                continue;
            }

            // Update Receive Time Point For Watchdog
            mReceiveTime = now;

            if (const auto *gateway = GetServer()->GetModule<UGateway>(); gateway != nullptr) {
                if (pkg->GetID() == 1001) {
                    gateway->OnHeartBeat(mPlayerID, pkg);
                } else {
                    gateway->OnClientPackage(mPlayerID, pkg);
                }
            }
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
        Disconnect();
    }
}

awaitable<void> UConnection::Watchdog() {
    if (mExpiration == ASteadyDuration::zero()) {
        co_return;
    }

    try {
        ASteadyTimePoint now;

        do {
            mWatchdog.expires_at(mReceiveTime + mExpiration);

            if (auto [ec] = co_await mWatchdog.async_wait(); ec) {
                SPDLOG_DEBUG("{:<20} - Timer Canceled.", __FUNCTION__);
                co_return;
            }

            now = std::chrono::steady_clock::now();
        } while ((mReceiveTime + mExpiration) > now);

        if (mSocket.is_open()) {
            SPDLOG_WARN("{:<20} - Watchdog Timeout", __FUNCTION__);
            Disconnect();
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
    }
}
