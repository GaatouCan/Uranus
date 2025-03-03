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
    : socket_(std::move(socket)),
      scene_(scene),
      codec_(nullptr),
      handler_(nullptr),
      watchdog_(socket_.get_executor()) {
}

Connection::~Connection() {
    Disconnect();
    spdlog::trace("{} - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
}

void Connection::ConnectToClient() {
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

void Connection::Disconnect() {
    GetWorld()->RemoveConnection(key_);

    if (socket_.is_open()) {
        socket_.close();

        // 保证OnClosed()回调只执行一次
        if (handler_ != nullptr && context_.has_value())
            handler_->OnClosed();
    }

    context_.reset();

    // 服务器关闭时数据包池不一定还在
    while (!output_.IsEmpty() && GetPackagePool()) {
        if (auto res = output_.PopFront(); res.has_value())
            GetPackagePool()->Recycle(res.value());
    }
}

int32_t Connection::GetSceneID() const {
    return scene_->GetSceneID();
}

Connection &Connection::SetContext(const std::any &ctx) {
    context_ = ctx;
    return *this;
}

Connection &Connection::ResetContext() {
    context_.reset();
    return *this;
}

Connection &Connection::SetKey(const std::string &key) {
    key_ = key;
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
    return scene_->GetThreadID();
}

PackagePool *Connection::GetPackagePool() const {
    return scene_->GetPackagePool();
}

MainScene *Connection::GetMainScene() const {
    return scene_;
}

GameWorld *Connection::GetWorld() const {
    return scene_->GetWorld();
}

bool Connection::IsSameThread() const {
    return GetThreadID() == std::this_thread::get_id();
}

IPackage *Connection::BuildPackage() const {
    return GetPackagePool()->Acquire();
}

void Connection::Send(IPackage *pkg) {
    const bool empty = output_.IsEmpty();
    output_.PushBack(pkg);

    if (empty)
        co_spawn(socket_.get_executor(), WritePackage(), detached);
}

asio::ip::address Connection::RemoteAddress() const {
    if (IsConnected())
        return socket_.remote_endpoint().address();
    return {};
}

awaitable<void> Connection::Watchdog() {
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

awaitable<void> Connection::WritePackage() {
    try {
        if (codec_ == nullptr) {
            spdlog::critical("{} - codec undefined - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
            Disconnect();
            co_return;
        }

        while (socket_.is_open() && !output_.IsEmpty()) {
            auto res = output_.PopFront();
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

awaitable<void> Connection::ReadPackage() {
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

awaitable<void> Connection::Timeout(const std::chrono::duration<uint32_t> expire) {
    SystemTimer timer(co_await asio::this_coro::executor);
    timer.expires_after(expire);
    co_await timer.async_wait();
}
