#include "Player.h"
#include "../common/ProtoType.h"

#include "GameWorld.h"
#include "impl/Package.h"
#include "system/event/EventSystem.h"

#include <utility>
#include <ranges>
#include <login.pb.h>


UPlayer::UPlayer(ConnectionPointer conn)
    : IBasePlayer(std::move(conn)),
      mComponentModule(this),
      mEventModule(this) {
}

UPlayer::~UPlayer() {
    spdlog::trace("{} - {}", __FUNCTION__, GetFullID());
}

UComponentModule &UPlayer::GetComponentModule() {
    return mComponentModule;
}

UEventModule &UPlayer::GetEventModule() {
    return mEventModule;
}

void UPlayer::OnDayChange() {
    if (!IsSameThread()) {
        RunInThread(&UPlayer::OnDayChange, this);
        return;
    }
    mComponentModule.OnDayChange();
}

awaitable<void> UPlayer::OnLogin() {
    mLoginTime = NowTimePoint();
    spdlog::info("{} - Player[{}] Login Successfully.", __FUNCTION__, GetFullID());

    // co_await mComponentModule.Deserialize();

    mComponentModule.OnLogin();

    Login::W2C_LoginResponse response;
    response.set_result(true);
    response.set_progress(100);
    response.set_describe("Component Load Completed");

    SEND_PACKAGE(this, W2C_LoginResponse, response)

    const auto param = new FEP_PlayerLogin;
    param->pid = GetFullID();

    DISPATCH_EVENT(PLAYER_LOGIN, param)

    co_return;
}

void UPlayer::OnLogout(const bool bForce, const std::string &otherAddress) {
    if (!IsSameThread()) {
        RunInThread(&UPlayer::OnLogout, this, bForce, otherAddress);
        return;
    }
    mLogoutTime = NowTimePoint();
    CleanAllTimer();

    // mComponentModule.Serialize();
    mComponentModule.OnLogout();
    spdlog::info("{} - Player[{}] Logout.", __FUNCTION__, GetFullID());

    if (bForce) {
        Login::W2C_ForceLogoutResponse res;
        res.set_player_id(GetFullID());
        res.set_address(otherAddress);

        SEND_PACKAGE(this, W2C_ForceLogoutResponse, res)
    }

    const auto param = new FEP_PlayerLogout;
    param->pid = GetFullID();

    DISPATCH_EVENT(PLAYER_LOGOUT, param);
}

bool UPlayer::IsOnline() const {
    constexpr TimePoint zeroTimePoint{};
    const auto now = NowTimePoint();

    return mLoginTime > zeroTimePoint && mLoginTime < now
           && (mLogoutTime > zeroTimePoint && mLoginTime < now)
           && mLogoutTime <= mLoginTime;
}


void UPlayer::Send(const int32_t id, const std::string_view data) const {
    const auto pkg = dynamic_cast<Package *>(BuildPackage());
    pkg->SetPackageID(id).SetData(data);

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(id)));
    SendPackage(pkg);
}

void UPlayer::Send(const int32_t id, const std::stringstream &ss) const {
    const auto pkg = dynamic_cast<Package *>(BuildPackage());
    pkg->SetPackageID(id).SetData(ss.str());

    spdlog::trace("{} - [{}]", __FUNCTION__, ProtoTypeToString(static_cast<protocol::EProtoType>(id)));
    SendPackage(pkg);
}

void UPlayer::SyncCache(FCacheNode *node) {
    mComponentModule.SyncCache(node);
}

void UPlayer::DispatchEvent(const EEvent event, IEventParam *param, const DispatchType type) {
    mEventModule.Dispatch(event, param, type);
}
