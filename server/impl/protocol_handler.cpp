#include "protocol_handler.h"
#include "player_id.h"
#include "../player/player.h"
#include "../player/player_manager.h"

#include "connection.h"
#include "game_world.h"
#include "system/manager/manager_system.h"

#include <spdlog/spdlog.h>


ProtocolHandler::ProtocolHandler(UProtocolRoute *route)
    : IProtocolHandler(route) {
}

void ProtocolHandler::Invoke(const ProtoFunctor &func, const std::shared_ptr<UConnection> &conn, IPackage *pkg) {
    const auto mgr = GET_MANAGER(PlayerManager);
    if (mgr == nullptr) {
        spdlog::critical("{} - PlayerManager not found", __FUNCTION__);
        return;
    }

    const auto pid = std::any_cast<FPlayerID>(conn->GetContext());
    if (const auto plr = mgr->FindPlayer(pid.local); plr != nullptr) {
        try {
            assert(plr->getConnection() == conn);
            std::invoke(func, plr, pkg);
        } catch (std::exception &e) {
            spdlog::error("{} - Player[{}] Invoke Protocol Handler Error: {}", __FUNCTION__, plr->getLocalID(), e.what());
        }
    } else {
        spdlog::warn("{} - Player[{}] Not Found", __FUNCTION__, pid.getLocalID());
    }
}

// awaitable<void> UProtocolHandlerImpl::InvokeCross(const ACrossFunctor &func, IPackage *pkg) {
//     co_await std::invoke(func, pkg);
// }
