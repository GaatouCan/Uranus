#include "ConnectionHandlerImpl.h"
#include "Connection.h"
#include "GameWorld.h"
#include "PlayerID.h"
#include "system/manager/ManagerSystem.h"

#include "../common/ProtoType.h"
#include "../player/PlayerManager.h"

UConnectionHandlerImpl::UConnectionHandlerImpl(UConnection *conn)
    : IConnectionHandler(conn){

}

void UConnectionHandlerImpl::OnConnected() {
}

void UConnectionHandlerImpl::OnClosed() {
    if (!conn_->GetContext().has_value())
        return;

    const auto pid = std::any_cast<FPlayerID>(conn_->GetContext());

    if (const auto mgr = GET_MANAGER(UPlayerManager);mgr != nullptr) {
        mgr->OnPlayerLogout(pid);
    }
}

awaitable<void> UConnectionHandlerImpl::OnReadPackage(IPackage *pkg) {
    spdlog::debug("{} - Receive Package[{}] From {}.", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(pkg->GetPackageID())), conn_->RemoteAddress().to_string());
    co_return;
}
