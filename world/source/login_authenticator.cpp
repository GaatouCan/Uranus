#include "../include/login_authenticator.h"

#include "../include/connection.h"
#include "../include/game_world.h"

#include "../include/scene/scene_manager.h"
#include "../include/scene/main_scene.h"

#include <spdlog/spdlog.h>

LoginAuthenticator::LoginAuthenticator(GameWorld *world)
    : mWorld(world) {
}

LoginAuthenticator::~LoginAuthenticator() {

}

void LoginAuthenticator::Init() {
    assert(mHandler != nullptr);
}

GameWorld * LoginAuthenticator::GetWorld() const {
    return mWorld;
}

bool LoginAuthenticator::VerifyAddress(const asio::ip::address &addr) {
    // TODO
    return true;
}

PlayerID LoginAuthenticator::VerifyToken(PlayerID pid, const std::string &token) {
    // TODO
    return pid;
}

awaitable<void> LoginAuthenticator::OnLogin(const ConnectionPointer &conn, IPackage *pkg) {
    if (conn == nullptr || pkg == nullptr)
        co_return;

    const auto info = co_await mHandler->ParseLoginInfo(pkg);
    if (!info.pid.IsAvailable()) {
        spdlog::warn("{} - Connection[{}] context is null but not receive the login request", __FUNCTION__, conn->GetKey());
        co_return;
    }

    spdlog::debug("{} - Player id: {}, token: {}", __FUNCTION__, info.pid.ToInt64(), info.token);
    if (const auto pid = VerifyToken(info.pid, info.token); pid.IsAvailable() && pid.cross == mWorld->GetServerID()) {
        conn->SetContext(std::make_any<PlayerID>(pid));

        if (const auto plr = co_await mHandler->OnPlayerLogin(conn, info); plr != nullptr) {
            if (const auto manager = mWorld->GetSceneManager(); manager != nullptr) {
                if (const auto scene = manager->GetScene(conn->GetSceneID()); scene != nullptr) {
                    scene->PlayerEnterScene(plr);
                    co_return;
                }
            }
        }
    } else if (!pid.IsAvailable())
        spdlog::error("{} - Player ID not available - key[{}]", __FUNCTION__, conn->GetKey());
    else if (pid.cross != mWorld->GetServerID())
        spdlog::error("{} - Server ID[{}] error, Connection Server ID[{}] - key[{}]", __FUNCTION__, mWorld->GetServerID(), pid.ToInt64(), conn->GetKey());

    conn->ResetContext();
    conn->Disconnect();
}

void LoginAuthenticator::AbortHandler() const {
    assert(mHandler != nullptr);
}
