#pragma once

#include "PackageCodec.h"


class UNetwork;
class UServer;


class BASE_API UConnection final : public std::enable_shared_from_this<UConnection> {

    using APackageChannel = TConcurrentChannel<void(std::error_code, shared_ptr<IPackageInterface>)>;

    UNetwork *module_;
    ATcpSocket socket_;

    unique_ptr<IPackageCodecBase> codec_;
    APackageChannel channel_;

    ASteadyTimer watchdog_;
    ASteadyTimePoint receiveTime_;
    ASteadyDuration expiration_;

    int64_t id_;
    std::atomic_int64_t playerId_;

public:
    UConnection() = delete;

    UConnection(UNetwork *module, ATcpSocket socket);
    ~UConnection();

    DISABLE_COPY_MOVE(UConnection)

    [[nodiscard]] ATcpSocket &GetSocket();
    [[nodiscard]] APackageChannel &GetChannel();

    template<class Type, class ... Args>
    requires std::derived_from<Type, IPackageCodecBase>
    void SetPackageCodec(Args && ... args);

    void SetExpireSecond(int sec);
    void SetPlayerID(int64_t id);

    void ConnectToClient();
    void Disconnect();

    [[nodiscard]] UNetwork *GetNetworkModule() const;
    [[nodiscard]] UServer *GetServer() const;

    shared_ptr<IPackageInterface> BuildPackage() const;

    asio::ip::address RemoteAddress() const;
    [[nodiscard]] int64_t GetConnectionID() const;
    [[nodiscard]] int64_t GetPlayerID() const;

    void SendPackage(const shared_ptr<IPackageInterface> &pkg);

private:
    awaitable<void> WritePackage();
    awaitable<void> ReadPackage();
    awaitable<void> Watchdog();
};

template<class Type, class ... Args>
requires std::derived_from<Type, IPackageCodecBase>
inline void UConnection::SetPackageCodec(Args && ... args) {
    codec_ = make_unique<Type>(socket_, std::forward<Args>(args)...);
}
