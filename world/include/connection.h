#pragma once

#include "package_codec.h"
#include "connection_handler.h"
#include "ts_deque.h"
#include "utils.h"

#include <asio/experimental/awaitable_operators.hpp>


using namespace std::literals::chrono_literals;
using namespace asio::experimental::awaitable_operators;


class BASE_API UConnection final : public std::enable_shared_from_this<UConnection> {

    ATcpSocket socket_;
    class UMainScene *scene_;

    std::unique_ptr<IPackageCodec>      codec_;
    std::unique_ptr<IConnectionHandler> handler_;

    TDeque<IPackage *> output_;

    std::string key_;

    ASystemTimer watchdog_;
    ATimePoint deadline_;

    std::any context_;
    uint32_t contextNullCount_ = 0;

    static std::chrono::duration<uint32_t> kExpireTime;
    static std::chrono::duration<uint32_t> kWriteTimeout;
    static std::chrono::duration<uint32_t> kReadTimeout;

    static constexpr int NULL_CONTEXT_MAX_COUNT = 3;

public:
    UConnection() = delete;

    UConnection(ATcpSocket socket, UMainScene *scene);
    ~UConnection();

    DISABLE_COPY_MOVE(UConnection)

    void connectToClient();
    void disconnect();

    [[nodiscard]] int32_t getSceneID() const;
    [[nodiscard]] AThreadID getThreadID() const;
    [[nodiscard]] IRecycler *getPackagePool() const;

    [[nodiscard]] UMainScene *getMainScene() const;
    [[nodiscard]] UGameWorld *getWorld() const;

    UConnection &setContext(const std::any &ctx);
    UConnection &resetContext();

    UConnection &setKey(const std::string &key);

    static void setWatchdogTimeout(uint32_t sec);
    static void setWriteTimeout(uint32_t sec);
    static void setReadTimeout(uint32_t sec);

    [[nodiscard]] bool checkSameThread() const;

    template<typename T, typename ... Args>
    requires std::derived_from<T, IPackageCodec>
    UConnection &setPackageCodec(Args &&... args) {
        if (codec_ != nullptr)
            codec_.reset();

        codec_ = std::make_unique<T>(weak_from_this(), std::forward<Args>(args)...);
        return *this;
    }

    template<typename T, typename ... Args>
    requires std::derived_from<T, IConnectionHandler>
    UConnection &setHandler(Args &&... args) {
        if (handler_ != nullptr)
            handler_.reset();

        handler_ = std::make_unique<T>(shared_from_this(), std::forward<Args>(args)...);
        return *this;
    }

    IPackage *buildPackage() const;

    void send(IPackage *pkg);

    [[nodiscard]] bool connected() const { return socket_.is_open(); }

    [[nodiscard]] std::string getKey() const { return key_; }
    [[nodiscard]] ATcpSocket &getSocket() { return socket_; }

    [[nodiscard]] const std::any &getContext() const { return context_; }
    [[nodiscard]] std::any getMutableContext() const { return context_; }

    [[nodiscard]] asio::ip::address remoteAddress() const;

private:
    awaitable<void> watchdog();

    awaitable<void> writePackage();
    awaitable<void> readPackage();

    static awaitable<void> Timeout(std::chrono::duration<uint32_t> expire);
};

using AConnectionPointer = std::shared_ptr<UConnection>;
