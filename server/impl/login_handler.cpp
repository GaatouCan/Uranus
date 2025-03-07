#include "login_handler.h"
#include "../common/proto_type.h"
#include "../player/player_manager.h"
#include "../player/player.h"

#include "impl/package.h"
#include "game_world.h"
#include "system/manager/manager_system.h"

#include <login.pb.h>
#include <spdlog/spdlog.h>


LoginHandler::LoginHandler(ULoginAuthenticator *owner)
    : ILoginHandler(owner) {
}

awaitable<std::shared_ptr<IBasePlayer>> LoginHandler::OnPlayerLogin(const std::shared_ptr<UConnection> &conn, const LoginInfo &info) {
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

awaitable<LoginInfo> LoginHandler::ParseLoginInfo(IPackage *pkg) {
    try {
        const auto tmp = dynamic_cast<FPackage *>(pkg);

        if (pkg->GetPackageID() != static_cast<uint32_t>(protocol::ProtoType::ClientLoginRequest))
            co_return LoginInfo{};

        Login::ClientLoginRequest request;
        request.ParseFromString(tmp->ToString());

        LoginInfo info;

        info.pid.FromInt64(request.player_id());
        info.token = request.token();

        co_return info;
    } catch (std::exception &e) {
        spdlog::warn("{} - {}", __FUNCTION__, e.what());
        co_return LoginInfo{};
    }
}
