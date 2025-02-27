#pragma once

#include "package_codec.h"
#include "connection_handler.h"
#include "thread_safe_deque.h"
#include "utils.h"

#include <asio/experimental/awaitable_operators.hpp>


using namespace std::literals::chrono_literals;
using namespace asio::experimental::awaitable_operators;


class BASE_API Connection final : public std::enable_shared_from_this<Connection> {

    TcpSocket socket_;
    class MainScene *scene_;

    std::unique_ptr<IPackageCodec>      codec_ = nullptr;
    std::unique_ptr<IConnectionHandler> handler_ = nullptr;

    ThreadSafeDeque<IPackage *> output_;

    std::string key_;

    SystemTimer watchdog_;
    TimePoint deadline_;

    std::any context_;
    uint32_t context_null_count_ = 0;

    static std::chrono::duration<uint32_t> kExpireTime;
    static std::chrono::duration<uint32_t> kWriteTimeout;
    static std::chrono::duration<uint32_t> kReadTimeout;

    static constexpr int NULL_CONTEXT_MAX_COUNT = 3;

public:
    Connection() = delete;

    Connection(TcpSocket socket, MainScene *scene);
    ~Connection();

    DISABLE_COPY_MOVE(Connection)

    void ConnectToClient();
    void Disconnect();

    [[nodiscard]] int32_t GetSceneID() const;
    [[nodiscard]] ThreadID GetThreadID() const;
    [[nodiscard]] class PackagePool *GetPackagePool() const;

    [[nodiscard]] MainScene *GetMainScene() const;
    [[nodiscard]] class GameWorld *GetWorld() const;

    Connection &SetContext(const std::any &ctx);
    Connection &ResetContext();

    Connection &SetKey(const std::string &key);

    static void SetWatchdogTimeout(uint32_t sec);
    static void SetWriteTimeout(uint32_t sec);
    static void SetReadTimeout(uint32_t sec);

    [[nodiscard]] bool IsSameThread() const;

    template<typename T>
    requires std::derived_from<T, IPackageCodec>
    Connection &SetPackageCodec() {
        if (codec_ != nullptr)
            codec_.reset();

        codec_ = std::make_unique<T>(this);
        return *this;
    }

    template<typename T>
    requires std::derived_from<T, IConnectionHandler>
    Connection &SetHandler() {
        if (handler_ != nullptr)
            handler_.reset();

        handler_ = std::make_unique<T>(this);
        return *this;
    }

    IPackage *BuildPackage() const;

    void Send(IPackage *pkg);

    [[nodiscard]] bool IsConnected() const { return socket_.is_open(); }

    [[nodiscard]] std::string GetKey() const { return key_; }
    [[nodiscard]] TcpSocket &GetSocket() { return socket_; }

    [[nodiscard]] const std::any &GetContext() const { return context_; }
    [[nodiscard]] std::any GetMutableContext() const { return context_; }

    [[nodiscard]] asio::ip::address RemoteAddress() const;

private:
    awaitable<void> Watchdog();

    awaitable<void> WritePackage();
    awaitable<void> ReadPackage();

    static awaitable<void> Timeout(std::chrono::duration<uint32_t> expire);
};

using ConnectionPointer = std::shared_ptr<Connection>;
