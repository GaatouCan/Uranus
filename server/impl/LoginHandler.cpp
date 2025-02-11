#include "LoginHandler.h"
#include "../common/ProtoType.h"
#include "../player/PlayerManager.h"
#include "../player/Player.h"

#include "impl/Package.h"
#include "GameWorld.h"
#include "system/manager/ManagerSystem.h"

#include <login.pb.h>
#include <spdlog/spdlog.h>


LoginHandler::LoginHandler(LoginAuthenticator *owner)
    : ILoginHandler(owner) {
}

awaitable<std::shared_ptr<IBasePlayer>> LoginHandler::OnPlayerLogin(const std::shared_ptr<Connection> &conn, const LoginInfo &info) {
    // if (const auto sys = GetWorld()->GetSystem<UManagerSystem>(); sys == nullptr) {
    //     spdlog::critical("{} - Manager System is null", __FUNCTION__);
    //     GetWorld()->Shutdown();
    //     exit(-1);
    // }

    const auto mgr = GET_MANAGER(UPlayerManager);
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
        const auto tmp = dynamic_cast<Package *>(pkg);

        if (pkg->GetPackageID() != static_cast<int32_t>(protocol::ProtoType::C2W_LoginRequest))
            co_return LoginInfo{};

        Login::C2W_LoginRequest request;
        request.ParseFromString(tmp->ToString());

        LoginInfo info;

        info.pid.FromUInt64(request.player_id());
        info.token = request.token();

        co_return info;
    } catch (std::exception &e) {
        spdlog::warn("{} - {}", __FUNCTION__, e.what());
        co_return LoginInfo{};
    }
}
