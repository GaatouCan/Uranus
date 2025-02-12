#include "connection_handler.h"
#include "connection.h"
#include "game_world.h"
#include "player_id.h"
#include "system/manager/manager_system.h"

#include "../common/proto_type.h"
#include "../player/player_manager.h"

ConnectionHandler::ConnectionHandler(Connection *conn)
    : IConnectionHandler(conn){

}

void ConnectionHandler::OnConnected() {
}

void ConnectionHandler::OnClosed() {
    if (!mConn->GetContext().has_value())
        return;

    const auto pid = std::any_cast<PlayerID>(mConn->GetContext());

    if (const auto mgr = GET_MANAGER(PlayerManager); mgr != nullptr) {
        mgr->OnPlayerLogout(pid);
    }
}

awaitable<void> ConnectionHandler::OnReadPackage(IPackage *pkg) {
    spdlog::debug("{} - Receive Package[{}] From {}.", __FUNCTION__, ProtoTypeToString(static_cast<protocol::ProtoType>(pkg->GetPackageID())), mConn->RemoteAddress().to_string());
    co_return;
}
