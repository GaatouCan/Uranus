#include "../include/LoginAuthenticator.h"

#include "../include/Connection.h"
#include "../include/GameWorld.h"

#include "../include/scene/SceneManager.h"
#include "../include/scene/MainScene.h"

#include "../include/GameWorld.h"

#include <spdlog/spdlog.h>

ULoginAuthenticator::ULoginAuthenticator(UGameWorld *world)
    : mWorld(world) {
}

ULoginAuthenticator::~ULoginAuthenticator() {

}

void ULoginAuthenticator::Init() {
}

UGameWorld * ULoginAuthenticator::GetWorld() const {
    return mWorld;
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

    // if (mHandler == nullptr) {
    //     spdlog::critical("{} - handler not set.", __FUNCTION__);
    //     mWorld->Shutdown();
    //     exit(-1);
    // }

    const auto info = co_await mHandler->ParseLoginInfo(pkg);
    if (!info.pid.IsAvailable()) {
        spdlog::warn("{} - Connection[{}] context is null but not receive the login request", __FUNCTION__, conn->GetKey());
        co_return;
    }

    spdlog::debug("{} - Player id: {}, token: {}", __FUNCTION__, info.pid.ToInt64(), info.token);
    if (const auto pid = VerifyToken(info.pid, info.token); pid.IsAvailable() && pid.cross == mWorld->GetServerID()) {
        conn->SetContext(std::make_any<FPlayerID>(pid));

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

void ULoginAuthenticator::AbortHandler() const {
    assert(mHandler != nullptr);
}
