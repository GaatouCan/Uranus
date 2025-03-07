#include "../include/login_authenticator.h"

#include "../include/connection.h"
#include "../include/game_world.h"

#include "../include/scene/scene_manager.h"
#include "../include/scene/main_scene.h"

#include <spdlog/spdlog.h>

ULoginAuthenticator::ULoginAuthenticator(UGameWorld *world)
    : world_(world) {
}

ULoginAuthenticator::~ULoginAuthenticator() {
    spdlog::info("{} - Shutdown.", __FUNCTION__);
}

void ULoginAuthenticator::Init() {
    assert(handler_ != nullptr);
}

UGameWorld * ULoginAuthenticator::GetWorld() const {
    return world_;
}

bool ULoginAuthenticator::VerifyAddress(const asio::ip::address &addr) {
    // TODO
    return true;
}

FPlayerID ULoginAuthenticator::VerifyToken(FPlayerID pid, const std::string &token) {
    // TODO
    return pid;
}

awaitable<void> ULoginAuthenticator::OnLogin(const AConnectionPointer &conn, IPackage *pkg) {
    if (conn == nullptr || pkg == nullptr)
        co_return;

    const auto info = co_await handler_->ParseLoginInfo(pkg);
    if (!info.pid.IsAvailable()) {
        spdlog::warn("{} - Connection[{}] context is null but not receive the login request", __FUNCTION__, conn->GetKey());
        co_return;
    }

    spdlog::debug("{} - Player id: {}, token: {}", __FUNCTION__, info.pid.ToInt64(), info.token);
    if (const auto pid = VerifyToken(info.pid, info.token); pid.IsAvailable() && pid.cross == world_->GetServerID()) {
        conn->SetContext(std::make_any<FPlayerID>(pid));

        if (const auto plr = co_await handler_->OnPlayerLogin(conn, info); plr != nullptr) {
            if (const auto manager = world_->GetSceneManager(); manager != nullptr) {
                if (const auto scene = manager->GetScene(conn->GetSceneID()); scene != nullptr) {
                    scene->PlayerEnterScene(plr);
                    co_return;
                }
            }
        }
    } else if (!pid.IsAvailable())
        spdlog::error("{} - Player ID not available - key[{}]", __FUNCTION__, conn->GetKey());
    else if (pid.cross != world_->GetServerID())
        spdlog::error("{} - Server ID[{}] error, Connection Server ID[{}] - key[{}]", __FUNCTION__, world_->GetServerID(), pid.ToInt64(), conn->GetKey());

    conn->ResetContext();
    conn->Disconnect();
}

void ULoginAuthenticator::AbortHandler() const {
    assert(handler_ != nullptr);
}
