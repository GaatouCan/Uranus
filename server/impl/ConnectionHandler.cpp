#include "ConnectionHandler.h"
#include "Connection.h"
#include "GameWorld.h"
#include "PlayerID.h"
#include "system/manager/ManagerSystem.h"

#include "../common/ProtoType.h"
#include "../player/PlayerManager.h"

ConnectionHandler::ConnectionHandler(Connection *conn)
    : IConnectionHandler(conn){

}

void ConnectionHandler::OnConnected() {
}

void ConnectionHandler::OnClosed() {
    if (!conn_->GetContext().has_value())
        return;

    const auto pid = std::any_cast<FPlayerID>(conn_->GetContext());

    if (const auto mgr = GET_MANAGER(UPlayerManager);mgr != nullptr) {
        mgr->OnPlayerLogout(pid);
    }
}

awaitable<void> ConnectionHandler::OnReadPackage(IPackage *pkg) {
    spdlog::debug("{} - Receive Package[{}] From {}.", __FUNCTION__, ProtoTypeToString(static_cast<protocol::ProtoType>(pkg->GetPackageID())), conn_->RemoteAddress().to_string());
    co_return;
}
