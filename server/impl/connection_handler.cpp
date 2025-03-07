#include "connection_handler.h"
#include "connection.h"
#include "game_world.h"
#include "player_id.h"
#include "system/manager/manager_system.h"

#include "../common/proto_type.h"
#include "../player/player_manager.h"

ConnectionHandler::ConnectionHandler(const std::weak_ptr<UConnection> &conn)
    : IConnectionHandler(conn){

}

void ConnectionHandler::OnConnected() {
}

void ConnectionHandler::OnClosed() {
    if (!conn_.lock()->GetContext().has_value())
        return;

    const auto pid = std::any_cast<FPlayerID>(conn_.lock()->GetContext());

    if (const auto mgr = GET_MANAGER(PlayerManager); mgr != nullptr) {
        mgr->OnPlayerLogout(pid);
    }
}

awaitable<void> ConnectionHandler::OnReadPackage(IPackage *pkg) {
    spdlog::debug("{} - Receive Package[{}] From {}.", __FUNCTION__, ProtoTypeToString(static_cast<protocol::ProtoType>(pkg->GetPackageID())), conn_.lock()->RemoteAddress().to_string());
    co_return;
}
