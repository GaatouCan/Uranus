#pragma once

#include "PackageCodec.h"
#include "ConnectionHandler.h"
#include "ThreadSafeDeque.h"
#include "utils.h"

#include <asio/experimental/awaitable_operators.hpp>


using namespace std::literals::chrono_literals;
using namespace asio::experimental::awaitable_operators;


class BASE_API UConnection final : public std::enable_shared_from_this<UConnection> {

    ATcpSocket socket_;
    class UMainScene *scene_;

    std::unique_ptr<IPackageCodec> codec_ = nullptr;
    std::unique_ptr<IConnectionHandler> handler_ = nullptr;

    TThreadSafeDeque<IPackage *> output_;

    std::string key_;

    ASystemTimer watchdogTimer_;
    ATimePoint deadline_;

    std::any ctx_;
    uint32_t contextNullCount_ = 0;

    static std::chrono::duration<uint32_t> expireTime;
    static std::chrono::duration<uint32_t> writeTimeout;
    static std::chrono::duration<uint32_t> readTimeout;

    static constexpr int NULL_CONTEXT_MAX_COUNT = 3;

public:
    UConnection() = delete;

    UConnection(ATcpSocket socket, UMainScene *scene);
    ~UConnection();

    DISABLE_COPY_MOVE(UConnection)

    void ConnectToClient();
    void Disconnect();

    [[nodiscard]] int32_t GetSceneID() const;
    [[nodiscard]] AThreadID GetThreadID() const;
    [[nodiscard]] class UPackagePool *GetPackagePool() const;

    [[nodiscard]] UMainScene *GetMainScene() const;
    [[nodiscard]] class UGameWorld *GetWorld() const;

    UConnection &SetContext(const std::any &ctx);
    UConnection &ResetContext();

    UConnection &SetKey(const std::string &key);

    static void SetWatchdogTimeout(uint32_t sec);
    static void SetWriteTimeout(uint32_t sec);
    static void SetReadTimeout(uint32_t sec);

    [[nodiscard]] bool IsSameThread() const;

    template<typename T>
    requires std::derived_from<T, IPackageCodec>
    UConnection &SetPackageCodec() {
        if (codec_ != nullptr)
            codec_.reset();

        codec_ = std::make_unique<T>(this);
        return *this;
    }

    template<typename T>
    requires std::derived_from<T, IConnectionHandler>
    UConnection &SetHandler() {
        if (handler_ != nullptr)
            handler_.reset();

        handler_ = std::make_unique<T>(this);
        return *this;
    }

    IPackage *BuildPackage() const;

    void Send(IPackage *pkg);

    [[nodiscard]] bool IsConnected() const { return socket_.is_open(); }

    [[nodiscard]] std::string GetKey() const { return key_; }
    [[nodiscard]] ATcpSocket &GetSocket() { return socket_; }

    [[nodiscard]] const std::any &GetContext() const { return ctx_; }
    [[nodiscard]] std::any GetMutableContext() const { return ctx_; }

    [[nodiscard]] asio::ip::address RemoteAddress() const;

private:
    awaitable<void> Watchdog();

    awaitable<void> WritePackage();
    awaitable<void> ReadPackage();

    static awaitable<void> Timeout(std::chrono::duration<uint32_t> expire);
};

using AConnectionPointer = std::shared_ptr<UConnection>;
