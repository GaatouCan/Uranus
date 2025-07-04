#pragma once

#include "../module.h"
#include "../utils.h"

#include <shared_mutex>
#include <absl/container/flat_hash_map.h>


class UConnection;
class IPackage;
class IRecycler;

using absl::flat_hash_map;
using AConnectionPointer = std::shared_ptr<UConnection>;

/**
 * The Network Module Of The Server;
 * With Independent asio::io_context To Handle I/O Event;
 * Manage All Client Connection
 */
class BASE_API UNetwork final : public IModule {

    DECLARE_MODULE(UNetwork)

protected:
    explicit UNetwork(UServer *server);

    void Initial() override;
    void Start() override;
    void Stop() override;

public:
    ~UNetwork() override;

    constexpr const char *GetModuleName() const override {
        return "Network Module";
    }

    /** Return The Independent asio::io_context Reference */
    [[nodiscard]] asio::io_context &GetIOContext();

    /** Find The Connection With Connection ID */
    AConnectionPointer FindConnection(int64_t cid) const;
    void RemoveConnection(int64_t cid, int64_t pid);

    void OnLoginSuccess(int64_t cid, int64_t pid, const std::shared_ptr<IPackage> &pkg) const;
    void OnLoginFailure(int64_t cid, const std::shared_ptr<IPackage> &pkg);

    /** Send The Package To The Client */
    void SendToClient(int64_t cid, const std::shared_ptr<IPackage> &pkg) const;

    /** Acquire One Package From The Internal Package Pool */
    std::shared_ptr<IPackage> BuildPackage() const;

private:
    awaitable<void> WaitForClient(uint16_t port);

private:
    asio::io_context mIOContext;
    ATcpAcceptor mAcceptor;

    /** Independent Thread To Run IO Context */
    std::thread mThread;

    /** Package Pool For I/O Data */
    std::shared_ptr<IRecycler> mPool;

    flat_hash_map<int64_t, AConnectionPointer> mConnectionMap;
    mutable std::shared_mutex mMutex;
};
