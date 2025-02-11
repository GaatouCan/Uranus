#include "ProtocolHandler.h"
#include "PlayerID.h"
#include "../player/Player.h"
#include "../player/PlayerManager.h"

#include "Connection.h"
#include "GameWorld.h"
#include "system/manager/ManagerSystem.h"

#include <spdlog/spdlog.h>


ProtocolHandler::ProtocolHandler(ProtocolRoute *route)
    : IProtocolHandler(route) {
}

void ProtocolHandler::Invoke(const ProtoFunctor &func, const std::shared_ptr<Connection> &conn, IPackage *pkg) {
    const auto mgr = GET_MANAGER(UPlayerManager);
    if (mgr == nullptr) {
        spdlog::critical("{} - PlayerManager not found", __FUNCTION__);
        return;
    }

    const auto pid = std::any_cast<PlayerID>(conn->GetContext());
    if (const auto plr = mgr->FindPlayer(pid.local); plr != nullptr) {
        try {
            assert(plr->GetConnection() == conn);
            std::invoke(func, plr, pkg);
        } catch (std::exception &e) {
            spdlog::error("{} - Player[{}] Invoke Protocol Handler Error: {}", __FUNCTION__, plr->GetLocalID(), e.what());
        }
    } else {
        spdlog::warn("{} - Player[{}] Not Found", __FUNCTION__, pid.GetLocalID());
    }
}

// awaitable<void> UProtocolHandlerImpl::InvokeCross(const ACrossFunctor &func, IPackage *pkg) {
//     co_await std::invoke(func, pkg);
// }
