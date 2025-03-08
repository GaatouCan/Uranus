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

void ULoginAuthenticator::init() {
    assert(handler_ != nullptr);
}

UGameWorld * ULoginAuthenticator::getWorld() const {
    return world_;
}

bool ULoginAuthenticator::verifyAddress(const asio::ip::address &addr) {
    // TODO
    return true;
}

FPlayerID ULoginAuthenticator::verifyToken(FPlayerID pid, const std::string &token) {
    // TODO
    return pid;
}

awaitable<void> ULoginAuthenticator::onLogin(const AConnectionPointer &conn, IPackage *pkg) {
    if (conn == nullptr || pkg == nullptr)
        co_return;

    const auto info = co_await handler_->parseLoginInfo(pkg);
    if (!info.pid.available()) {
        spdlog::warn("{} - Connection[{}] context is null but not receive the login request", __FUNCTION__, conn->getKey());
        co_return;
    }

    spdlog::debug("{} - Player id: {}, token: {}", __FUNCTION__, info.pid.toInt64(), info.token);
    if (const auto pid = verifyToken(info.pid, info.token); pid.available() && pid.cross == world_->getServerID()) {
        conn->setContext(std::make_any<FPlayerID>(pid));

        if (const auto plr = co_await handler_->onPlayerLogin(conn, info); plr != nullptr) {
            if (const auto manager = world_->getSceneManager(); manager != nullptr) {
                if (const auto scene = manager->getScene(conn->getSceneID()); scene != nullptr) {
                    scene->playerEnterScene(plr);
                    co_return;
                }
            }
        }
    } else if (!pid.available())
        spdlog::error("{} - Player ID not available - key[{}]", __FUNCTION__, conn->getKey());
    else if (pid.cross != world_->getServerID())
        spdlog::error("{} - Server ID[{}] error, Connection Server ID[{}] - key[{}]", __FUNCTION__, world_->getServerID(), pid.toInt64(), conn->getKey());

    conn->resetContext();
    conn->disconnect();
}

void ULoginAuthenticator::abort() const {
    assert(handler_ != nullptr);
}
