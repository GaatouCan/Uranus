#include "Connection.h"
#include "Network.h"
#include "../Server.h"
#include "../Gateway/Gateway.h"
#include "../Login/LoginAuth.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>


using namespace asio::experimental::awaitable_operators;
using namespace std::literals::chrono_literals;

UConnection::UConnection(UNetwork *module, ATcpSocket socket)
    : mModule(module),
      mSocket(std::move(socket)),
      mCodec(nullptr),
      mWatchdog(mSocket.get_executor()),
      mExpiration(std::chrono::seconds(30)),
      mPlayerID(-1),
      mLastLoginTime(0),
      bDisconnected(false) {
    mID = static_cast<int64_t>(mSocket.native_handle());
}

UConnection::~UConnection() {
    Disconnect();
}

ATcpSocket &UConnection::GetSocket() {
    return mSocket;
}

void UConnection::SetExpireSecond(const int sec) {
    mExpiration = std::chrono::seconds(sec);
}

void UConnection::SetPlayerID(const int64_t id) {
    if (mPlayerID > 0)
        return;

    mPlayerID = id;
    mLastLoginTime = 0;
}

void UConnection::ConnectToClient() {
    assert(mCodec != nullptr);

    mReceivedTime = NowTimePoint();

    co_spawn(mSocket.get_executor(), [self = shared_from_this()]() -> awaitable<void> {
        co_await (self->ReadPackage() || self->Watchdog());
    }, detached);
}

void UConnection::Disconnect() {
    if (bDisconnected)
        return;

    bDisconnected = true;

    mModule->RemoveConnection(mID, mPlayerID);

    if (mSocket.is_open()) {
        mSocket.close();
    }

    mOutput.Clear();
    mWatchdog.cancel();

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

std::shared_ptr<IPackage> UConnection::BuildPackage() const {
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

void UConnection::SendPackage(const std::shared_ptr<IPackage> &pkg) {
    if (pkg == nullptr)
        return;

    const auto bEmpty = mOutput.IsEmpty();
    mOutput.PushBack(pkg);

    if (bEmpty) {
        co_spawn(mSocket.get_executor(), WritePackage(), detached);
    }
}

int64_t UConnection::GetLastLoginTime() const {
    return mLastLoginTime;
}

awaitable<void> UConnection::WritePackage() {
    try {
        while (mSocket.is_open() && !mOutput.IsEmpty()) {
            const auto pkg = mOutput.PopFront();
            if (pkg == nullptr) {
                co_return;
            }

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
            if (pkg == nullptr) {
                co_return;
            }

            if (const auto ret = co_await mCodec->Decode(pkg); !ret) {
                SPDLOG_WARN("{:<20} - Failed To Read Package", __FUNCTION__);
                Disconnect();
                break;
            }

            mReceivedTime = NowTimePoint();

            if (mPlayerID < 0) {
                const auto now = utils::UnixTime();
                // 最近5秒内请求登录过
                if (now - mLastLoginTime.load() < 5)
                    continue;

                --mPlayerID;
                mLastLoginTime = now;

                if (mPlayerID < -3) {
                    SPDLOG_WARN("{:<20} - Connection[{}] Try Login Too Many Times",
                        __FUNCTION__, RemoteAddress().to_string());
                    Disconnect();
                    break;
                }

                // Handle Login Logic
                if (auto *login = GetServer()->GetModule<ULoginAuth>(); login != nullptr) {
                    login->OnPlayerLogin(mID, pkg);
                }
                continue;
            }

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
    if (mExpiration == ATimePoint::duration::zero()) {
        co_return;
    }

    try {
        ATimePoint now;

        do {
            mWatchdog.expires_at(mReceivedTime + mExpiration);

            if (auto [ec] = co_await mWatchdog.async_wait(); ec) {
                SPDLOG_DEBUG("{:<20} - Timer Canceled.", __FUNCTION__);
                co_return;
            }

            now = NowTimePoint();
        } while ((mReceivedTime + mExpiration) > now);

        if (mSocket.is_open()) {
            SPDLOG_WARN("{:<20} - Watchdog Timeout", __FUNCTION__);
            Disconnect();
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
    }
}
