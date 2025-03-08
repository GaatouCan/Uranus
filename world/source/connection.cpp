#include "../include/connection.h"
#include "../include/game_world.h"
#include "../include/package_pool.h"
#include "../include/login_authenticator.h"
#include "../include/protocol_route.h"
#include "../include/scene/main_scene.h"

#include <spdlog/spdlog.h>


std::chrono::duration<uint32_t> UConnection::kExpireTime = 30s;
std::chrono::duration<uint32_t> UConnection::kWriteTimeout = 10s;
std::chrono::duration<uint32_t> UConnection::kReadTimeout = 10s;

UConnection::UConnection(ATcpSocket socket, UMainScene *scene)
    : socket_(std::move(socket)),
      scene_(scene),
      codec_(nullptr),
      handler_(nullptr),
      watchdog_(socket_.get_executor()) {
}

UConnection::~UConnection() {
    Disconnect();
    spdlog::trace("{} - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
}

void UConnection::ConnectToClient() {
    assert(codec_ != nullptr && handler_ != nullptr);

    deadline_ = NowTimePoint() + kExpireTime;

    spdlog::debug("{} - Connection from {} run in thread: {}", __FUNCTION__, RemoteAddress().to_string(), utils::ThreadIdToInt(GetThreadID()));
    if (handler_ != nullptr)
        handler_->OnConnected();

    co_spawn(socket_.get_executor(), [self = shared_from_this()]() mutable -> awaitable<void> {
        try {
            co_await (self->ReadPackage() || self->Watchdog());
        } catch (std::exception &e) {
            spdlog::warn("UConnection::ConnectToClient() - {}", e.what());
        }
    }, detached);
}

void UConnection::Disconnect() {
    GetWorld()->RemoveConnection(key_);

    if (socket_.is_open()) {
        socket_.close();

        // 保证OnClosed()回调只执行一次
        if (handler_ != nullptr && context_.has_value())
            handler_->OnClosed();
    }

    context_.reset();

    // 服务器关闭时数据包池不一定还在
    while (!output_.empty() && GetPackagePool()) {
        if (auto res = output_.popFront(); res.has_value())
            GetPackagePool()->Recycle(res.value());
    }
}

int32_t UConnection::GetSceneID() const {
    return scene_->GetSceneID();
}

UConnection &UConnection::SetContext(const std::any &ctx) {
    context_ = ctx;
    return *this;
}

UConnection &UConnection::ResetContext() {
    context_.reset();
    return *this;
}

UConnection &UConnection::SetKey(const std::string &key) {
    key_ = key;
    return *this;
}

void UConnection::SetWatchdogTimeout(const uint32_t sec) {
    kExpireTime = std::chrono::seconds(sec);
}

void UConnection::SetWriteTimeout(const uint32_t sec) {
    kWriteTimeout = std::chrono::seconds(sec);
}

void UConnection::SetReadTimeout(const uint32_t sec) {
    kReadTimeout = std::chrono::seconds(sec);
}

AThreadID UConnection::GetThreadID() const {
    return scene_->GetThreadID();
}

UPackagePool *UConnection::GetPackagePool() const {
    return scene_->GetPackagePool();
}

UMainScene *UConnection::GetMainScene() const {
    return scene_;
}

UGameWorld *UConnection::GetWorld() const {
    return scene_->GetWorld();
}

bool UConnection::IsSameThread() const {
    return GetThreadID() == std::this_thread::get_id();
}

IPackage *UConnection::BuildPackage() const {
    return GetPackagePool()->Acquire();
}

void UConnection::Send(IPackage *pkg) {
    const bool empty = output_.empty();
    output_.pushBack(pkg);

    if (empty)
        co_spawn(socket_.get_executor(), WritePackage(), detached);
}

asio::ip::address UConnection::RemoteAddress() const {
    if (IsConnected())
        return socket_.remote_endpoint().address();
    return {};
}

awaitable<void> UConnection::Watchdog() {
    try {
        decltype(deadline_) now;
        do {
            watchdog_.expires_at(deadline_);
            co_await watchdog_.async_wait();
            now = NowTimePoint();

            if (context_null_count_ != -1) {
                if (!context_.has_value())
                    context_null_count_++;
                else
                    context_null_count_ = -1;
            }
        } while (deadline_ > now && context_null_count_ < NULL_CONTEXT_MAX_COUNT);

        if (socket_.is_open()) {
            spdlog::warn("{} - watchdog Timer timeout - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
            Disconnect();
        }
    } catch (std::exception &e) {
        spdlog::warn("{} - {} - key[{}]", __FUNCTION__, e.what(), key_.empty() ? "null" : key_);
    }
}

awaitable<void> UConnection::WritePackage() {
    try {
        if (codec_ == nullptr) {
            spdlog::critical("{} - codec undefined - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
            Disconnect();
            co_return;
        }

        while (socket_.is_open() && !output_.empty()) {
            auto res = output_.popFront();
            if (!res.has_value())
                continue;

            const auto pkg = res.value();
            co_await codec_->Encode(pkg);

            if (pkg->IsAvailable()) {
                if (handler_ != nullptr)
                    co_await handler_->OnWritePackage(pkg);

                GetPackagePool()->Recycle(pkg);
            } else {
                spdlog::warn("{} - Write Failed - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
                GetPackagePool()->Recycle(pkg);
                Disconnect();
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {} - key[{}]", __FUNCTION__, e.what(), key_.empty() ? "null" : key_);
        Disconnect();
    }
}

awaitable<void> UConnection::ReadPackage() {
    try {
        if (codec_ == nullptr) {
            spdlog::error("{} - PackageCodec Undefined - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
            Disconnect();
            co_return;
        }

        while (socket_.is_open()) {
            const auto pkg = BuildPackage();

            co_await codec_->Decode(pkg);

            if (pkg->IsAvailable()) {
                deadline_ = NowTimePoint() + kExpireTime;

                if (handler_ != nullptr)
                    co_await handler_->OnReadPackage(pkg);

                if (!context_.has_value())
                    co_await GetWorld()->GetLoginAuthenticator()->OnLogin(shared_from_this(), pkg);
                else
                    GetWorld()->GetProtocolRoute()->OnReadPackage(shared_from_this(), pkg);
            } else {
                spdlog::warn("{} - Read failed - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
                Disconnect();
            }

            GetPackagePool()->Recycle(pkg);
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {} - key[{}]", __FUNCTION__, e.what(), key_.empty() ? "null" : key_);
        Disconnect();
    }
}

awaitable<void> UConnection::Timeout(const std::chrono::duration<uint32_t> expire) {
    ASystemTimer timer(co_await asio::this_coro::executor);
    timer.expires_after(expire);
    co_await timer.async_wait();
}
