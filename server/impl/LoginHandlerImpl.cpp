#include "LoginHandlerImpl.h"
#include "../common/ProtoType.h"
#include "../player/PlayerManager.h"
#include "../player/Player.h"

#include "impl/Package.h"
#include "GameWorld.h"
#include "system/manager/ManagerSystem.h"

#include <login.pb.h>
#include <spdlog/spdlog.h>


ULoginHandlerImpl::ULoginHandlerImpl(LoginAuthenticator *owner)
    : ILoginHandler(owner) {
}

awaitable<std::shared_ptr<IBasePlayer>> ULoginHandlerImpl::OnPlayerLogin(const std::shared_ptr<Connection> &conn, const FLoginInfo &info) {
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

awaitable<FLoginInfo> ULoginHandlerImpl::ParseLoginInfo(IPackage *pkg) {
    try {
        const auto tmp = dynamic_cast<Package *>(pkg);

        if (pkg->GetPackageID() != static_cast<int32_t>(protocol::EProtoType::C2W_LoginRequest))
            co_return FLoginInfo{};

        Login::C2W_LoginRequest request;
        request.ParseFromString(tmp->ToString());

        FLoginInfo info;

        info.pid.FromUInt64(request.player_id());
        info.token = request.token();

        co_return info;
    } catch (std::exception &e) {
        spdlog::warn("{} - {}", __FUNCTION__, e.what());
        co_return FLoginInfo{};
    }
}
