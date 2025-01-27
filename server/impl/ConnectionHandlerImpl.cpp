#include "ConnectionHandlerImpl.h"
#include "Connection.h"
#include "PlayerID.h"

#include "../common/ProtoType.h"

UConnectionHandlerImpl::UConnectionHandlerImpl(UConnection *conn)
    : IConnectionHandler(conn){
}

void UConnectionHandlerImpl::OnConnected() {
}

void UConnectionHandlerImpl::OnClosed() {
    if (!mConn->GetContext().has_value())
        return;

    const auto pid = std::any_cast<FPlayerID>(mConn->GetContext());

    // if (const auto mgr = UPlayerManager::Instance(); mgr != nullptr) {
    //     mgr->OnPlayerLogout(pid);
    // }
}

awaitable<void> UConnectionHandlerImpl::OnReadPackage(IPackage *pkg) {
    spdlog::debug("{} - Receive Package[{}] From {}.", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(pkg->GetPackageID())), mConn->RemoteAddress().to_string());
    co_return;
}
