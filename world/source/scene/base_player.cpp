#include "../../include/scene/base_player.h"
#include "scene/base_scene.h"
// #include "../../CrossRoute.h"

#include <utility>
#include <ranges>

IBasePlayer::IBasePlayer(ConnectionPointer conn)
    : mOwner(nullptr),
      mConn(std::move(conn)),
      mPlayerID(std::any_cast<PlayerID>(mConn->GetContext())) {

}

IBasePlayer::~IBasePlayer() {
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

bool IBasePlayer::SetConnection(ConnectionPointer conn) {
    if (std::any_cast<PlayerID>(mConn->GetContext()) != mPlayerID) {
        return false;
    }
    mConn = std::move(conn);
    return true;
}

ConnectionPointer IBasePlayer::GetConnection() const {
    return mConn;
}

TcpSocket &IBasePlayer::GetSocket() const {
    return mConn->GetSocket();
}

asio::io_context &IBasePlayer::GetIOContext() const {
    const auto ctx = GetSocket().get_executor().target<asio::io_context>();
    return *const_cast<asio::io_context *>(ctx);
}

GameWorld * IBasePlayer::GetWorld() const {
    return mConn->GetWorld();
}

ThreadID IBasePlayer::GetThreadID() const {
    return mConn->GetThreadID();
}

bool IBasePlayer::IsSameThread() const {
    return mConn->IsSameThread();
}

int32_t IBasePlayer::GetLocalID() const {
    return mPlayerID.GetLocalID();
}

int32_t IBasePlayer::GetCrossID() const {
    return mPlayerID.GetCrossID();
}

const PlayerID &IBasePlayer::GetPlayerID() const {
    return mPlayerID;
}

int64_t IBasePlayer::GetFullID() const {
    return mPlayerID.ToInt64();
}

IPackage *IBasePlayer::BuildPackage() const {
    return mConn->BuildPackage();
}

void IBasePlayer::SendPackage(IPackage *pkg) const {
    // if (UCrossRoute::Instance().IsCrossProtocol(pkg->GetPackageID())) {
    //     UCrossRoute::Instance().Send(pkg);
    //     return;
    // }

    mConn->Send(pkg);
}

void IBasePlayer::OnEnterScene(IBaseScene *scene) {
    if (mOwner != nullptr) {
        // TODO
    }
    mOwner = scene;
    mEnterTime = NowTimePoint();
}

void IBasePlayer::OnLeaveScene(IBaseScene *scene) {
    if (mOwner == nullptr)
        return;

    mLeaveTime = NowTimePoint();
    mOwner = nullptr;
}

bool IBasePlayer::TryLeaveScene() {
    if (mOwner == nullptr) {
        return false;
    }
    mOwner->PlayerLeaveScene(shared_from_this());
    return true;
}

bool IBasePlayer::IsInScene(const int32_t id) const {
    if (mOwner == nullptr)
        return false;

    if (id == 0)
        return true;

    return mOwner->GetSceneID() == id;
}

int32_t IBasePlayer::GetCurrentSceneID() const {
    if (mOwner == nullptr)
        return -1;
    return mOwner->GetSceneID();
}

IBaseScene *IBasePlayer::GetCurrentScene() const {
    return mOwner;
}

TimePoint IBasePlayer::GetEnterSceneTime() const {
    return mEnterTime;
}

TimePoint IBasePlayer::GetLeaveSceneTime() const {
    return mLeaveTime;
}

void IBasePlayer::SetPlatformInfo(const PlatformInfo &platform) {
    mPlatform = platform;
}

const PlatformInfo &IBasePlayer::GetPlatformInfo() const {
    return mPlatform;
}

bool IBasePlayer::StopTimer(const UniqueID &tid) {
    const auto timer = GetTimer(tid);
    if (timer == nullptr)
        return false;

    timer->Stop();
    return true;
}

void IBasePlayer::CleanAllTimer() {
    std::unique_lock lock(mTimerMutex);
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

RepeatedTimer *IBasePlayer::GetTimer(const UniqueID &tid) {
    std::shared_lock lock(mTimerMutex);
    if (const auto it = mTimerMap.find(tid); it != mTimerMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<UniqueID> IBasePlayer::AddTimer(RepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    UniqueID timerID = UniqueID::RandomGenerate();

    {
        std::shared_lock lock(mTimerMutex);
        while (mTimerMap.contains(timerID)) {
            timerID = UniqueID::RandomGenerate();
        }
    }

    {
        std::unique_lock lock(mTimerMutex);
        mTimerMap[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([weak = weak_from_this()](const UniqueID &tid) mutable {
        if (const auto self = weak.lock()) {
            self->RemoveTimer(tid);
        }
    });
    return timerID;
}

bool IBasePlayer::RemoveTimer(const UniqueID &tid) {
    std::unique_lock lock(mTimerMutex);
    if (const auto it = mTimerMap.find(tid); it != mTimerMap.end()) {
        const auto timer = it->second;
        mTimerMap.erase(it);

        if (timer != nullptr) {
            delete timer;
            return true;
        }
    }
    return false;
}
