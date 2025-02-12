#include "../include/connection.h"
#include "../include/game_world.h"
#include "../include/package_pool.h"
#include "../include/login_authenticator.h"
#include "../include/protocol_route.h"
#include "../include/scene/main_scene.h"

#include <spdlog/spdlog.h>


std::chrono::duration<uint32_t> Connection::kExpireTime = 30s;
std::chrono::duration<uint32_t> Connection::kWriteTimeout = 10s;
std::chrono::duration<uint32_t> Connection::kReadTimeout = 10s;

Connection::Connection(TcpSocket socket, MainScene *scene)
    : mSocket(std::move(socket)),
      mScene(scene),
      mWatchdogTimer(mSocket.get_executor()) {
}

Connection::~Connection() {
    Disconnect();
    spdlog::trace("{} - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
}

void Connection::ConnectToClient() {
    assert(mCodec != nullptr && mHandler != nullptr);

    mDeadline = NowTimePoint() + kExpireTime;

    spdlog::debug("{} - Connection from {} run in thread: {}", __FUNCTION__, RemoteAddress().to_string(), utils::ThreadIdToInt(GetThreadID()));
    if (mHandler != nullptr)
        mHandler->OnConnected();

    co_spawn(mSocket.get_executor(), [self = shared_from_this()]() mutable -> awaitable<void> {
        try {
            co_await (self->ReadPackage() || self->Watchdog());
        } catch (std::exception &e) {
            spdlog::warn("UConnection::ConnectToClient() - {}", e.what());
        }
    }, detached);
}

void Connection::Disconnect() {
    GetWorld()->RemoveConnection(mKey);

    if (mSocket.is_open()) {
        mSocket.close();

        // 保证OnClosed()回调只执行一次
        if (mHandler != nullptr && mContext.has_value())
            mHandler->OnClosed();
    }

    mContext.reset();

    // 服务器关闭时数据包池不一定还在
    while (!mOutput.IsEmpty() && GetPackagePool()) {
        if (auto res = mOutput.PopFront(); res.has_value())
            GetPackagePool()->Recycle(res.value());
    }
}

int32_t Connection::GetSceneID() const {
    return mScene->GetSceneID();
}

Connection &Connection::SetContext(const std::any &ctx) {
    mContext = ctx;
    return *this;
}

Connection &Connection::ResetContext() {
    mContext.reset();
    return *this;
}

Connection &Connection::SetKey(const std::string &key) {
    mKey = key;
    return *this;
}

void Connection::SetWatchdogTimeout(const uint32_t sec) {
    kExpireTime = std::chrono::seconds(sec);
}

void Connection::SetWriteTimeout(const uint32_t sec) {
    kWriteTimeout = std::chrono::seconds(sec);
}

void Connection::SetReadTimeout(const uint32_t sec) {
    kReadTimeout = std::chrono::seconds(sec);
}

ThreadID Connection::GetThreadID() const {
    return mScene->GetThreadID();
}

PackagePool *Connection::GetPackagePool() const {
    return mScene->GetPackagePool();
}

MainScene *Connection::GetMainScene() const {
    return mScene;
}

GameWorld *Connection::GetWorld() const {
    return mScene->GetWorld();
}

bool Connection::IsSameThread() const {
    return GetThreadID() == std::this_thread::get_id();
}

IPackage *Connection::BuildPackage() const {
    return GetPackagePool()->Acquire();
}

void Connection::Send(IPackage *pkg) {
    const bool bEmpty = mOutput.IsEmpty();
    mOutput.PushBack(pkg);

    if (bEmpty)
        co_spawn(mSocket.get_executor(), WritePackage(), detached);
}

asio::ip::address Connection::RemoteAddress() const {
    if (IsConnected())
        return mSocket.remote_endpoint().address();
    return {};
}

awaitable<void> Connection::Watchdog() {
    try {
        decltype(mDeadline) now;
        do {
            mWatchdogTimer.expires_at(mDeadline);
            co_await mWatchdogTimer.async_wait();
            now = NowTimePoint();

            if (mContextNullCount != -1) {
                if (!mContext.has_value())
                    mContextNullCount++;
                else
                    mContextNullCount = -1;
            }
        } while (mDeadline > now && mContextNullCount < NULL_CONTEXT_MAX_COUNT);

        if (mSocket.is_open()) {
            spdlog::warn("{} - watchdog Timer timeout - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
            Disconnect();
        }
    } catch (std::exception &e) {
        spdlog::warn("{} - {} - key[{}]", __FUNCTION__, e.what(), mKey.empty() ? "null" : mKey);
    }
}

awaitable<void> Connection::WritePackage() {
    try {
        if (mCodec == nullptr) {
            spdlog::critical("{} - codec undefined - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
            Disconnect();
            co_return;
        }

        while (mSocket.is_open() && !mOutput.IsEmpty()) {
            auto res = mOutput.PopFront();
            if (!res.has_value())
                continue;

            const auto pkg = res.value();
            co_await mCodec->Encode(pkg);

            if (pkg->IsAvailable()) {
                if (mHandler != nullptr)
                    co_await mHandler->OnWritePackage(pkg);

                GetPackagePool()->Recycle(pkg);
            } else {
                spdlog::warn("{} - Write Failed - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
                GetPackagePool()->Recycle(pkg);
                Disconnect();
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {} - key[{}]", __FUNCTION__, e.what(), mKey.empty() ? "null" : mKey);
        Disconnect();
    }
}

awaitable<void> Connection::ReadPackage() {
    try {
        if (mCodec == nullptr) {
            spdlog::error("{} - PackageCodec Undefined - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
            Disconnect();
            co_return;
        }

        while (mSocket.is_open()) {
            const auto pkg = BuildPackage();

            co_await mCodec->Decode(pkg);

            if (pkg->IsAvailable()) {
                mDeadline = NowTimePoint() + kExpireTime;

                if (mHandler != nullptr)
                    co_await mHandler->OnReadPackage(pkg);

                if (!mContext.has_value())
                    co_await GetWorld()->GetLoginAuthenticator()->OnLogin(shared_from_this(), pkg);
                else
                    GetWorld()->GetProtocolRoute()->OnReadPackage(shared_from_this(), pkg);
            } else {
                spdlog::warn("{} - Read failed - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
                Disconnect();
            }

            GetPackagePool()->Recycle(pkg);
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {} - key[{}]", __FUNCTION__, e.what(), mKey.empty() ? "null" : mKey);
        Disconnect();
    }
}

awaitable<void> Connection::Timeout(const std::chrono::duration<uint32_t> expire) {
    SystemTimer timer(co_await asio::this_coro::executor);
    timer.expires_after(expire);
    co_await timer.async_wait();
}
