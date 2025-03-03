#include "player.h"
#include "cache_node.h"
#include "../common/proto_type.h"

#include "game_world.h"
#include "impl/package.h"
#include "system/event/event_system.h"

#include <utility>
#include <ranges>
#include <login.pb.h>


Player::Player(const ConnectionPointer &conn)
    : IBasePlayer(conn),
      component_module_(this),
      event_module_(this) {
}

Player::~Player() {
    spdlog::trace("{} - {}", __FUNCTION__, GetFullID());
}


void Player::OnDayChange() {
    if (!IsSameThread()) {
        RunInThread(&Player::OnDayChange, this);
        return;
    }
    component_module_.OnDayChange();
}

awaitable<void> Player::OnLogin() {
    login_time_ = NowTimePoint();
    spdlog::info("{} - Player[{}] Login Successfully.", __FUNCTION__, GetFullID());

    Login::LoginResponse response;
    response.set_result(true);
    response.set_progress(100);
    response.set_describe("Component Load Completed");

    SEND_PACKAGE(this, LoginResponse, response)

    co_await component_module_.Deserialize();
    component_module_.OnLogin();

    const auto param = new EP_PlayerLogin;
    param->pid = GetFullID();

    DISPATCH_EVENT(PLAYER_LOGIN, param)

    co_return;
}

void Player::OnLogout(const bool is_force, const std::string &other_address) {
    if (!IsSameThread()) {
        RunInThread(&Player::OnLogout, this, is_force, other_address);
        return;
    }
    logout_time_ = NowTimePoint();
    CleanAllTimer();

    component_module_.OnLogout();
    spdlog::info("{} - Player[{}] Logout.", __FUNCTION__, GetFullID());

    component_module_.Serialize();

    if (is_force) {
        Login::ForceLogoutResponse res;
        res.set_player_id(GetFullID());
        res.set_address(other_address);

        SEND_PACKAGE(this, ForceLogoutResponse, res)
    }

    const auto param = new EP_PlayerLogout;
    param->pid = GetFullID();

    DISPATCH_EVENT(PLAYER_LOGOUT, param);
}

bool Player::IsOnline() const {
    constexpr TimePoint zero_time_point{};
    const auto now = NowTimePoint();

    return login_time_ > zero_time_point && login_time_ < now
           && (logout_time_ > zero_time_point && login_time_ < now)
           && logout_time_ <= login_time_;
}


void Player::Send(const uint32_t id, const std::string_view data) const {
    const auto pkg = dynamic_cast<Package *>(BuildPackage());
    pkg->SetPackageID(id).SetData(data);

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::ProtoType>(id)));
    SendPackage(pkg);
}

void Player::Send(const uint32_t id, const std::stringstream &ss) const {
    const auto pkg = dynamic_cast<Package *>(BuildPackage());
    pkg->SetPackageID(id).SetData(ss.str());

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::ProtoType>(id)));
    SendPackage(pkg);
}

void Player::SyncCache(CacheNode *node) {
    node->pid = GetFullID();
    node->lastLoginTime = utils::ToUnixTime(login_time_);
    if (logout_time_.time_since_epoch().count() > 0) {
        node->lastLogoutTime = utils::ToUnixTime(logout_time_);
    }
    component_module_.SyncCache(node);
}

void Player::DispatchEvent(const Event event, IEventParam *param, const DispatchType type) {
    event_module_.Dispatch(event, param, type);
}
