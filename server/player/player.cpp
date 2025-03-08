#include "player.h"
#include "cache_node.h"
#include "../common/proto_type.h"

#include "game_world.h"
#include "impl/package.h"
#include "system/event/event_system.h"

#include <utility>
#include <ranges>
#include <login.pb.h>


Player::Player(const AConnectionPointer &conn)
    : IBasePlayer(conn),
      component_module_(this),
      event_module_(this) {
}

Player::~Player() {
    spdlog::trace("{} - {}", __FUNCTION__, getFullID());
}


void Player::OnDayChange() {
    if (!isSameThread()) {
        runInThread(&Player::OnDayChange, this);
        return;
    }
    component_module_.OnDayChange();
}

awaitable<void> Player::OnLogin() {
    login_time_ = NowTimePoint();
    spdlog::info("{} - Player[{}] Login Successfully.", __FUNCTION__, getFullID());

    Login::LoginResponse response;
    response.set_result(true);
    response.set_progress(100);
    response.set_describe("Component Load Completed");

    SEND_PACKAGE(this, LoginResponse, response)

    co_await component_module_.Deserialize();
    component_module_.OnLogin();

    const auto param = new EP_PlayerLogin;
    param->pid = getFullID();

    DISPATCH_EVENT(PLAYER_LOGIN, param)

    co_return;
}

void Player::OnLogout(const bool is_force, const std::string &other_address) {
    if (!isSameThread()) {
        runInThread(&Player::OnLogout, this, is_force, other_address);
        return;
    }
    logout_time_ = NowTimePoint();
    cleanAllTimer();

    component_module_.OnLogout();
    spdlog::info("{} - Player[{}] Logout.", __FUNCTION__, getFullID());

    component_module_.Serialize();

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

bool Player::IsOnline() const {
    constexpr ATimePoint zero_time_point{};
    const auto now = NowTimePoint();

    return login_time_ > zero_time_point && login_time_ < now
           && (logout_time_ > zero_time_point && login_time_ < now)
           && logout_time_ <= login_time_;
}


void Player::Send(const uint32_t id, const std::string_view data) const {
    const auto pkg = dynamic_cast<FPackage *>(buildPackage());
    pkg->setPackageID(id).setData(data);

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(id)));
    sendPackage(pkg);
}

void Player::Send(const uint32_t id, const std::stringstream &ss) const {
    const auto pkg = dynamic_cast<FPackage *>(buildPackage());
    pkg->setPackageID(id).setData(ss.str());

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(id)));
    sendPackage(pkg);
}

void Player::SyncCache(CacheNode *node) {
    node->pid = getFullID();
    node->lastLoginTime = utils::ToUnixTime(login_time_);
    if (logout_time_.time_since_epoch().count() > 0) {
        node->lastLogoutTime = utils::ToUnixTime(logout_time_);
    }
    component_module_.SyncCache(node);
}

void Player::DispatchEvent(const EEvent event, IEventParam *param, const EDispatchType type) {
    event_module_.Dispatch(event, param, type);
}
