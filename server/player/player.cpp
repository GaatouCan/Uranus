#include "player.h"
#include "cache_node.h"
#include "../common/proto_type.h"

#include "game_world.h"
#include "impl/package.h"
#include "system/event/event_system.h"

#include <utility>
#include <ranges>
#include <login.pb.h>


UPlayer::UPlayer(const AConnectionPointer &conn)
    : IBasePlayer(conn),
      componentModule_(this),
      eventModule_(this) {
}

UPlayer::~UPlayer() {
    spdlog::trace("{} - {}", __FUNCTION__, getFullID());
}


void UPlayer::onDayChange() {
    if (!isSameThread()) {
        runInThread(&UPlayer::onDayChange, this);
        return;
    }
    componentModule_.onDayChange();
}

awaitable<void> UPlayer::onLogin() {
    loginTime_ = NowTimePoint();
    spdlog::info("{} - Player[{}] Login Successfully.", __FUNCTION__, getFullID());

    Login::LoginResponse response;
    response.set_result(true);
    response.set_progress(100);
    response.set_describe("Component Load Completed");

    SEND_PACKAGE(this, LoginResponse, response)

    co_await componentModule_.deserialize();
    componentModule_.onLogin();

    const auto param = new EP_PlayerLogin;
    param->pid = getFullID();

    DISPATCH_EVENT(PLAYER_LOGIN, param)

    co_return;
}

void UPlayer::onLogout(const bool is_force, const std::string &other_address) {
    if (!isSameThread()) {
        runInThread(&UPlayer::onLogout, this, is_force, other_address);
        return;
    }
    logoutTime_ = NowTimePoint();
    cleanAllTimer();

    componentModule_.onLogout();
    spdlog::info("{} - Player[{}] Logout.", __FUNCTION__, getFullID());

    componentModule_.serialize();

    if (is_force) {
        Login::ForceLogoutResponse res;
        res.set_player_id(getFullID());
        res.set_address(other_address);

        SEND_PACKAGE(this, ForceLogoutResponse, res)
    }

    const auto param = new EP_PlayerLogout;
    param->pid = getFullID();

    DISPATCH_EVENT(PLAYER_LOGOUT, param);
}

bool UPlayer::isOnline() const {
    constexpr ATimePoint zero_time_point{};
    const auto now = NowTimePoint();

    return loginTime_ > zero_time_point && loginTime_ < now
           && (logoutTime_ > zero_time_point && loginTime_ < now)
           && logoutTime_ <= loginTime_;
}


void UPlayer::send(const uint32_t id, const std::string_view data) const {
    const auto pkg = dynamic_cast<FPackage *>(buildPackage());
    pkg->setPackageID(id).setData(data);

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(id)));
    sendPackage(pkg);
}

void UPlayer::send(const uint32_t id, const std::stringstream &ss) const {
    const auto pkg = dynamic_cast<FPackage *>(buildPackage());
    pkg->setPackageID(id).setData(ss.str());

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(id)));
    sendPackage(pkg);
}

void UPlayer::syncCache(CacheNode *node) {
    node->pid = getFullID();
    node->lastLoginTime = utils::ToUnixTime(loginTime_);
    if (logoutTime_.time_since_epoch().count() > 0) {
        node->lastLogoutTime = utils::ToUnixTime(logoutTime_);
    }
    componentModule_.syncCache(node);
}

void UPlayer::dispatchEvent(const EEvent event, IEventParam *param, const EDispatchType type) {
    eventModule_.dispatch(event, param, type);
}
