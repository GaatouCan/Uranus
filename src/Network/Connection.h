#pragma once

#include "ConcurrentDeque.h"
#include "PackageCodec.h"


class UNetwork;
class UServer;

class BASE_API UConnection final : public std::enable_shared_from_this<UConnection> {

    UNetwork *mModule;
    ATcpSocket mSocket;

    std::unique_ptr<IPackageCodec> mCodec;
    TConcurrentDeque<std::shared_ptr<IPackage>> mOutput;

    ASystemTimer mWatchdog;
    ATimePoint mReceivedTime;
    ATimePoint::duration mExpiration;

    int64_t mID;
    std::atomic_int64_t mPlayerID;
    std::atomic_int64_t mLastLoginTime;

    std::atomic_bool bDisconnected;

public:
    UConnection() = delete;

    UConnection(UNetwork *module, ATcpSocket socket);
    ~UConnection();

    DISABLE_COPY_MOVE(UConnection)

    [[nodiscard]] ATcpSocket &GetSocket();

    template<class Type, class ... Args>
    requires std::derived_from<Type, IPackageCodec>
    void SetPackageCodec(Args && ... args);

    void SetExpireSecond(int sec);
    void SetPlayerID(int64_t id);

    void ConnectToClient();
    void Disconnect();

    [[nodiscard]] UNetwork *GetNetworkModule() const;
    [[nodiscard]] UServer *GetServer() const;

    std::shared_ptr<IPackage> BuildPackage() const;

    asio::ip::address RemoteAddress() const;
    [[nodiscard]] int64_t GetConnectionID() const;
    [[nodiscard]] int64_t GetPlayerID() const;

    void SendPackage(const std::shared_ptr<IPackage> &pkg);

    [[nodiscard]] int64_t GetLastLoginTime() const;

private:
    awaitable<void> WritePackage();
    awaitable<void> ReadPackage();
    awaitable<void> Watchdog();
};

template<class Type, class ... Args>
requires std::derived_from<Type, IPackageCodec>
inline void UConnection::SetPackageCodec(Args && ... args) {
    mCodec = std::unique_ptr<Type>(std::forward<Args>(args)...);
}
