#include "player.h"
#include "../common/proto_type.h"

#include "game_world.h"
#include "impl/package.h"
#include "system/event/event_system.h"

#include <utility>
#include <ranges>
#include <login.pb.h>


Player::Player(ConnectionPointer conn)
    : IBasePlayer(std::move(conn)),
      mComponentModule(this),
      mEventModule(this) {
}

Player::~Player() {
    spdlog::trace("{} - {}", __FUNCTION__, GetFullID());
}


void Player::OnDayChange() {
    if (!IsSameThread()) {
        RunInThread(&Player::OnDayChange, this);
        return;
    }
    mComponentModule.OnDayChange();
}

awaitable<void> Player::OnLogin() {
    mLoginTime = NowTimePoint();
    spdlog::info("{} - Player[{}] Login Successfully.", __FUNCTION__, GetFullID());

    Login::LoginResponse response;
    response.set_result(true);
    response.set_progress(100);
    response.set_describe("Component Load Completed");

    SEND_PACKAGE(this, LoginResponse, response)

    co_await mComponentModule.Deserialize();
    mComponentModule.OnLogin();

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
    mLogoutTime = NowTimePoint();
    CleanAllTimer();

    // mComponentModule.Serialize();
    mComponentModule.OnLogout();
    spdlog::info("{} - Player[{}] Logout.", __FUNCTION__, GetFullID());

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

    return mLoginTime > zero_time_point && mLoginTime < now
           && (mLogoutTime > zero_time_point && mLoginTime < now)
           && mLogoutTime <= mLoginTime;
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
    mComponentModule.SyncCache(node);
}

void Player::DispatchEvent(const Event event, IEventParam *param, const DispatchType type) {
    mEventModule.Dispatch(event, param, type);
}
