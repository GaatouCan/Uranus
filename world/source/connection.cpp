#include "../include/connection.h"
#include "../include/game_world.h"
#include "../include/recycler.h"
#include "../include/login_authenticator.h"
#include "../include/proto_route.h"
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
    disconnect();
    spdlog::trace("{} - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
}

void UConnection::connectToClient() {
    assert(codec_ != nullptr && handler_ != nullptr);

    deadline_ = NowTimePoint() + kExpireTime;

    spdlog::debug("{} - Connection from {} run in thread: {}", __FUNCTION__, remoteAddress().to_string(), utils::ThreadIdToInt(getThreadID()));
    if (handler_ != nullptr)
        handler_->onConnected();

    co_spawn(socket_.get_executor(), [self = shared_from_this()]() mutable -> awaitable<void> {
        try {
            co_await (self->readPackage() || self->watchdog());
        } catch (std::exception &e) {
            spdlog::warn("UConnection::ConnectToClient() - {}", e.what());
        }
    }, detached);
}

void UConnection::disconnect() {
    getWorld()->removeConnection(key_);

    if (socket_.is_open()) {
        socket_.close();

        // 保证OnClosed()回调只执行一次
        if (handler_ != nullptr && context_.has_value())
            handler_->onClosed();
    }

    context_.reset();

    // 服务器关闭时数据包池不一定还在
    while (!output_.empty() && getPackagePool()) {
        if (auto res = output_.popFront(); res.has_value())
            getPackagePool()->recycle(res.value());
    }
}

int32_t UConnection::getSceneID() const {
    return scene_->getSceneID();
}

UConnection &UConnection::setContext(const std::any &ctx) {
    context_ = ctx;
    return *this;
}

UConnection &UConnection::resetContext() {
    context_.reset();
    return *this;
}

UConnection &UConnection::setKey(const std::string &key) {
    key_ = key;
    return *this;
}

void UConnection::setWatchdogTimeout(const uint32_t sec) {
    kExpireTime = std::chrono::seconds(sec);
}

void UConnection::setWriteTimeout(const uint32_t sec) {
    kWriteTimeout = std::chrono::seconds(sec);
}

void UConnection::setReadTimeout(const uint32_t sec) {
    kReadTimeout = std::chrono::seconds(sec);
}

AThreadID UConnection::getThreadID() const {
    return scene_->getThreadID();
}

IRecycler *UConnection::getPackagePool() const {
    return scene_->getPackagePool();
}

UMainScene *UConnection::getMainScene() const {
    return scene_;
}

UGameWorld *UConnection::getWorld() const {
    return scene_->getWorld();
}

bool UConnection::isSameThread() const {
    return getThreadID() == std::this_thread::get_id();
}

IPackage *UConnection::buildPackage() const {
    return dynamic_cast<IPackage *>(getPackagePool()->acquire());
}

void UConnection::send(IPackage *pkg) {
    const bool empty = output_.empty();
    output_.pushBack(pkg);

    if (empty)
        co_spawn(socket_.get_executor(), writePackage(), detached);
}

asio::ip::address UConnection::remoteAddress() const {
    if (connected())
        return socket_.remote_endpoint().address();
    return {};
}

awaitable<void> UConnection::watchdog() {
    try {
        decltype(deadline_) now;
        do {
            watchdog_.expires_at(deadline_);
            co_await watchdog_.async_wait();
            now = NowTimePoint();

            if (contextNullCount_ != -1) {
                if (!context_.has_value())
                    contextNullCount_++;
                else
                    contextNullCount_ = -1;
            }
        } while (deadline_ > now && contextNullCount_ < NULL_CONTEXT_MAX_COUNT);

        if (socket_.is_open()) {
            spdlog::warn("{} - watchdog Timer timeout - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
            disconnect();
        }
    } catch (std::exception &e) {
        spdlog::warn("{} - {} - key[{}]", __FUNCTION__, e.what(), key_.empty() ? "null" : key_);
    }
}

awaitable<void> UConnection::writePackage() {
    try {
        if (codec_ == nullptr) {
            spdlog::critical("{} - codec undefined - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
            disconnect();
            co_return;
        }

        while (socket_.is_open() && !output_.empty()) {
            auto res = output_.popFront();
            if (!res.has_value())
                continue;

            const auto pkg = res.value();
            co_await codec_->encode(pkg);

            if (pkg->available()) {
                if (handler_ != nullptr)
                    co_await handler_->onWritePackage(pkg);

                getPackagePool()->recycle(pkg);
            } else {
                spdlog::warn("{} - Write Failed - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
                getPackagePool()->recycle(pkg);
                disconnect();
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {} - key[{}]", __FUNCTION__, e.what(), key_.empty() ? "null" : key_);
        disconnect();
    }
}

awaitable<void> UConnection::readPackage() {
    try {
        if (codec_ == nullptr) {
            spdlog::error("{} - PackageCodec Undefined - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
            disconnect();
            co_return;
        }

        while (socket_.is_open()) {
            const auto pkg = buildPackage();

            co_await codec_->decode(pkg);

            if (pkg->available()) {
                deadline_ = NowTimePoint() + kExpireTime;

                if (handler_ != nullptr)
                    co_await handler_->onReadPackage(pkg);

                if (!context_.has_value())
                    co_await getWorld()->getLoginAuthenticator()->onLogin(shared_from_this(), pkg);
                else
                    getWorld()->getProtoRoute()->onReadPackage(shared_from_this(), pkg);
            } else {
                spdlog::warn("{} - Read failed - key[{}]", __FUNCTION__, key_.empty() ? "null" : key_);
                disconnect();
            }

            getPackagePool()->recycle(pkg);
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {} - key[{}]", __FUNCTION__, e.what(), key_.empty() ? "null" : key_);
        disconnect();
    }
}

awaitable<void> UConnection::Timeout(const std::chrono::duration<uint32_t> expire) {
    ASystemTimer timer(co_await asio::this_coro::executor);
    timer.expires_after(expire);
    co_await timer.async_wait();
}
