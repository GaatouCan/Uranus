#pragma once

#include "PackageCodec.h"
#include "ConnectionHandler.h"
#include "ThreadSafeDeque.h"
#include "utils.h"

#include <asio/experimental/awaitable_operators.hpp>


using namespace std::literals::chrono_literals;
using namespace asio::experimental::awaitable_operators;


class BASE_API UConnection final : public std::enable_shared_from_this<UConnection> {

    ATcpSocket mSocket;
    class UMainScene *mScene;

    std::unique_ptr<IPackageCodec> mCodec = nullptr;
    std::unique_ptr<IConnectionHandler> mHandler = nullptr;

    TThreadSafeDeque<IPackage *> mOutput;

    std::string mKey;

    ASystemTimer mWatchdogTimer;
    ATimePoint mDeadline;

    std::any mContext;
    uint32_t mContextNullCount = 0;

    static std::chrono::duration<uint32_t> sExpireTime;
    static std::chrono::duration<uint32_t> sWriteTimeout;
    static std::chrono::duration<uint32_t> sReadTimeout;

    static constexpr int kNullContextMaxCount = 3;

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
    [[nodiscard]] bool HasCodecSet() const;

    template<typename T>
    requires std::derived_from<T, IPackageCodec>
    UConnection &SetPackageCodec() {
        if (mCodec != nullptr)
            mCodec.reset();

        mCodec = std::make_unique<T>(this);
        return *this;
    }

    [[nodiscard]] bool HasHandlerSet() const;

    template<typename T>
    requires std::derived_from<T, IConnectionHandler>
    UConnection &SetHandler() {
        if (mHandler != nullptr)
            mHandler.reset();

        mHandler = std::make_unique<T>(this);
        return *this;
    }

    IPackage *BuildPackage() const;

    void Send(IPackage *pkg);

    [[nodiscard]] bool IsConnected() const { return mSocket.is_open(); }

    [[nodiscard]] std::string GetKey() const { return mKey; }
    [[nodiscard]] ATcpSocket &GetSocket() { return mSocket; }

    [[nodiscard]] const std::any &GetContext() const { return mContext; }
    [[nodiscard]] std::any GetMutableContext() const { return mContext; }

    [[nodiscard]] asio::ip::address RemoteAddress() const;

private:
    awaitable<void> Watchdog();

    awaitable<void> WritePackage();
    awaitable<void> ReadPackage();

    static awaitable<void> Timeout(std::chrono::duration<uint32_t> expire);
};

using AConnectionPointer = std::shared_ptr<UConnection>;
