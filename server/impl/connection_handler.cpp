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

void ConnectionHandler::onConnected() {
}

void ConnectionHandler::onClosed() {
    if (!conn_.lock()->getContext().has_value())
        return;

    const auto pid = std::any_cast<FPlayerID>(conn_.lock()->getContext());

    if (const auto mgr = GET_MANAGER(PlayerManager); mgr != nullptr) {
        mgr->OnPlayerLogout(pid);
    }
}

awaitable<void> ConnectionHandler::onReadPackage(IPackage *pkg) {
    spdlog::debug("{} - Receive Package[{}] From {}.", __FUNCTION__, ProtoTypeToString(static_cast<protocol::ProtoType>(pkg->getPackageID())), conn_.lock()->remoteAddress().to_string());
    co_return;
}
