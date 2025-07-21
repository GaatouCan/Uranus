#pragma once

#include "Module.h"
#include "Types.h"

#include <shared_mutex>
#include <absl/container/flat_hash_map.h>


class UConnection;
class IPackageInterface;
class IRecyclerBase;

using absl::flat_hash_map;

/**
 * The Network Module Of The Server;
 * With Independent asio::io_context To Handle I/O Event;
 * Manage All Client Connection
 */
class BASE_API UNetwork final : public IModuleBase {

    DECLARE_MODULE(UNetwork)

protected:
    UNetwork();

    void Initial() override;
    void Start() override;
    void Stop() override;

public:
    ~UNetwork() override;

    constexpr const char *GetModuleName() const override {
        return "Network Module";
    }

    /** Return The Independent asio::io_context Reference */
    [[nodiscard]] io_context &GetIOContext();

    /** Find The Connection With Connection ID */
    shared_ptr<UConnection> FindConnection(int64_t cid) const;
    void RemoveConnection(int64_t cid, int64_t pid);

    void OnLoginSuccess(int64_t cid, int64_t pid, const shared_ptr<IPackageInterface> &pkg) const;
    void OnLoginFailure(int64_t cid, const shared_ptr<IPackageInterface> &pkg);

    /** Send The Package To The Client */
    void SendToClient(int64_t cid, const shared_ptr<IPackageInterface> &pkg) const;

    /** Acquire One Package From The Internal Package Pool */
    shared_ptr<IPackageInterface> BuildPackage() const;

private:
    awaitable<void> WaitForClient(uint16_t port);

private:
    io_context mIOContext;
    ATcpAcceptor mAcceptor;

    /** Independent Thread To Run IO Context */
    std::thread mThread;

    /** Package Pool For I/O Data */
    shared_ptr<IRecyclerBase> mPool;

    flat_hash_map<int64_t, shared_ptr<UConnection>> mConnectionMap;
    mutable std::shared_mutex mMutex;
};
