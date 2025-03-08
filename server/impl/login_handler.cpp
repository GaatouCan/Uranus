#include "login_handler.h"
#include "../common/proto_type.h"
#include "../player/player_manager.h"
#include "../player/player.h"

#include "impl/package.h"
#include "game_world.h"
#include "system/manager/manager_system.h"

#include <login.pb.h>
#include <spdlog/spdlog.h>


ULoginHandler::ULoginHandler(ULoginAuthenticator *owner)
    : ILoginHandler(owner) {
}

awaitable<std::shared_ptr<IBasePlayer>> ULoginHandler::onPlayerLogin(const std::shared_ptr<UConnection> &conn, const FLoginInfo &info) {
    const auto mgr = GET_MANAGER(PlayerManager);
    if (mgr == nullptr) {
        spdlog::critical("{} - Player Manager is null", __FUNCTION__);
        co_return nullptr;
    }

    try {
        auto plr = co_await mgr->OnPlayerLogin(conn, info.pid);
        co_return plr;
    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
        co_return nullptr;
    }
}

awaitable<FLoginInfo> ULoginHandler::parseLoginInfo(IPackage *pkg) {
    try {
        const auto tmp = dynamic_cast<FPackage *>(pkg);

        if (pkg->getPackageID() != static_cast<uint32_t>(protocol::EProtoType::ClientLoginRequest))
            co_return FLoginInfo{};

        Login::ClientLoginRequest request;
        request.ParseFromString(tmp->toString());

        FLoginInfo info;

        info.pid.fromInt64(request.player_id());
        info.token = request.token();

        co_return info;
    } catch (std::exception &e) {
        spdlog::warn("{} - {}", __FUNCTION__, e.what());
        co_return FLoginInfo{};
    }
}
