#include "Player.h"
#include "../common/ProtoType.h"

#include "GameWorld.h"
#include "impl/Package.h"
#include "system/event/EventSystem.h"

#include <utility>
#include <ranges>
#include <login.pb.h>


Player::Player(ConnectionPointer conn)
    : IBasePlayer(std::move(conn)),
      component_module_(this),
      event_module_(this) {
}

Player::~Player() {
    spdlog::trace("{} - {}", __FUNCTION__, GetFullID());
}

ComponentModule &Player::GetComponentModule() {
    return component_module_;
}

EventModule &Player::GetEventModule() {
    return event_module_;
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

    // co_await mComponentModule.Deserialize();

    component_module_.OnLogin();

    Login::W2C_LoginResponse response;
    response.set_result(true);
    response.set_progress(100);
    response.set_describe("Component Load Completed");

    SEND_PACKAGE(this, W2C_LoginResponse, response)

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

    // mComponentModule.Serialize();
    component_module_.OnLogout();
    spdlog::info("{} - Player[{}] Logout.", __FUNCTION__, GetFullID());

    if (is_force) {
        Login::W2C_ForceLogoutResponse res;
        res.set_player_id(GetFullID());
        res.set_address(other_address);

        SEND_PACKAGE(this, W2C_ForceLogoutResponse, res)
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
    component_module_.SyncCache(node);
}

void Player::DispatchEvent(const Event event, IEventParam *param, const DispatchType type) {
    event_module_.Dispatch(event, param, type);
}
