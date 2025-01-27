#include "../include/Connection.h"
#include "../include/PackagePool.h"
#include "../include/scene/MainScene.h"
#include "../include/GameWorld.h"
#include "../include/LoginAuthenticator.h"
#include "../include/ProtocolRoute.h"

#include <spdlog/spdlog.h>


std::chrono::duration<uint32_t> UConnection::sExpireTime = 30s;
std::chrono::duration<uint32_t> UConnection::sWriteTimeout = 10s;
std::chrono::duration<uint32_t> UConnection::sReadTimeout = 10s;


UConnection::UConnection(ATcpSocket socket, UMainScene *scene)
    : mSocket(std::move(socket)),
      mScene(scene),
      mWatchdogTimer(mSocket.get_executor()) {
}

UConnection::~UConnection() {
    Disconnect();
    spdlog::trace("{} - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
}

void UConnection::ConnectToClient() {
    mDeadline = NowTimePoint() + sExpireTime;

    spdlog::debug("{} - Connection from {} run in thread: {}", __FUNCTION__, RemoteAddress().to_string(),
                  utils::ThreadIdToInt(GetThreadID()));
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

void UConnection::Disconnect() {
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
        GetPackagePool()->Recycle(mOutput.PopFront());
    }
}

int32_t UConnection::GetSceneID() const {
    return mScene->GetSceneID();
}

UConnection &UConnection::SetContext(const std::any &ctx) {
    mContext = ctx;
    return *this;
}

UConnection &UConnection::ResetContext() {
    mContext.reset();
    return *this;
}

UConnection &UConnection::SetKey(const std::string &key) {
    mKey = key;
    return *this;
}

void UConnection::SetWatchdogTimeout(const uint32_t sec) {
    sExpireTime = std::chrono::seconds(sec);
}

void UConnection::SetWriteTimeout(const uint32_t sec) {
    sWriteTimeout = std::chrono::seconds(sec);
}

void UConnection::SetReadTimeout(const uint32_t sec) {
    sReadTimeout = std::chrono::seconds(sec);
}

AThreadID UConnection::GetThreadID() const {
    return mScene->GetThreadID();
}

UPackagePool *UConnection::GetPackagePool() const {
    return mScene->GetPackagePool();
}

UMainScene *UConnection::GetMainScene() const {
    return mScene;
}

UGameWorld *UConnection::GetWorld() const {
    return mScene->GetWorld();
}

bool UConnection::IsSameThread() const {
    return GetThreadID() == std::this_thread::get_id();
}

bool UConnection::HasCodecSet() const {
    return mCodec != nullptr;
}

bool UConnection::HasHandlerSet() const {
    return mHandler != nullptr;
}

IPackage *UConnection::BuildPackage() const {
    return GetPackagePool()->Acquire();
}

void UConnection::Send(IPackage *pkg) {
    const bool bEmpty = mOutput.IsEmpty();
    mOutput.PushBack(pkg);

    if (bEmpty)
        co_spawn(mSocket.get_executor(), WritePackage(), detached);
}

asio::ip::address UConnection::RemoteAddress() const {
    if (IsConnected())
        return mSocket.remote_endpoint().address();
    return {};
}

awaitable<void> UConnection::Watchdog() {
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
        } while (mDeadline > now && mContextNullCount < kNullContextMaxCount);

        if (mSocket.is_open()) {
            spdlog::warn("{} - watchdog Timer timeout - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
            Disconnect();
        }
    } catch (std::exception &e) {
        spdlog::warn("{} - {} - key[{}]", __FUNCTION__, e.what(), mKey.empty() ? "null" : mKey);
    }
}

awaitable<void> UConnection::WritePackage() {
    try {
        if (mCodec == nullptr) {
            spdlog::critical("{} - codec undefined - key[{}]", __FUNCTION__, mKey.empty() ? "null" : mKey);
            Disconnect();
            co_return;
        }

        while (mSocket.is_open() && !mOutput.IsEmpty()) {
            const auto pkg = mOutput.PopFront();

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

awaitable<void> UConnection::ReadPackage() {
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
                mDeadline = NowTimePoint() + sExpireTime;

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

awaitable<void> UConnection::Timeout(const std::chrono::duration<uint32_t> expire) {
    ASystemTimer timer(co_await asio::this_coro::executor);
    timer.expires_after(expire);
    co_await timer.async_wait();
}
