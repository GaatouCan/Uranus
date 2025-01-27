#include "../../include/scene/BasePlayer.h"
#include "scene/AbstractScene.h"
// #include "../../CrossRoute.h"

#include <utility>
#include <ranges>

IBasePlayer::IBasePlayer(AConnectionPointer conn)
    : mOwnerScene(nullptr),
      mConn(std::move(conn)),
      mPlayerID(std::any_cast<FPlayerID>(mConn->GetContext())) {

}

IBasePlayer::~IBasePlayer() {
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

bool IBasePlayer::SetConnection(AConnectionPointer conn) {
    if (std::any_cast<FPlayerID>(mConn->GetContext()) != mPlayerID) {
        return false;
    }
    mConn = std::move(conn);
    return true;
}

AConnectionPointer IBasePlayer::GetConnection() const {
    return mConn;
}

ATcpSocket &IBasePlayer::GetSocket() const {
    return mConn->GetSocket();
}

asio::io_context &IBasePlayer::GetIOContext() const {
    const auto ctx = GetSocket().get_executor().target<asio::io_context>();
    return *const_cast<asio::io_context *>(ctx);
}

UGameWorld * IBasePlayer::GetWorld() const {
    return mConn->GetWorld();
}

AThreadID IBasePlayer::GetThreadID() const {
    return mConn->GetThreadID();
}

bool IBasePlayer::IsSameThread() const {
    return mConn->IsSameThread();
}

uint32_t IBasePlayer::GetLocalID() const {
    return mPlayerID.GetLocalID();
}

uint32_t IBasePlayer::GetCrossID() const {
    return mPlayerID.GetCrossID();
}

const FPlayerID &IBasePlayer::GetPlayerID() const {
    return mPlayerID;
}

uint64_t IBasePlayer::GetFullID() const {
    return mPlayerID.ToUInt64();
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

void IBasePlayer::OnEnterScene(IAbstractScene *scene) {
    if (mOwnerScene != nullptr) {
        // TODO
    }
    mOwnerScene = scene;
    mEnterTime = NowTimePoint();
}

void IBasePlayer::OnLeaveScene(IAbstractScene *scene) {
    if (mOwnerScene == nullptr)
        return;

    mLeaveTime = NowTimePoint();
    mOwnerScene = nullptr;
}

bool IBasePlayer::TryLeaveScene() {
    if (mOwnerScene == nullptr) {
        return false;
    }
    mOwnerScene->PlayerLeaveScene(shared_from_this());
    return true;
}

bool IBasePlayer::IsInScene(const int32_t id) const {
    if (mOwnerScene == nullptr)
        return false;

    if (id == 0)
        return true;

    return mOwnerScene->GetSceneID() == id;
}

int32_t IBasePlayer::GetCurrentSceneID() const {
    if (mOwnerScene == nullptr)
        return -1;
    return mOwnerScene->GetSceneID();
}

IAbstractScene *IBasePlayer::GetCurrentScene() const {
    return mOwnerScene;
}

ATimePoint IBasePlayer::GetEnterSceneTime() const {
    return mEnterTime;
}

ATimePoint IBasePlayer::GetLeaveSceneTime() const {
    return mLeaveTime;
}

void IBasePlayer::SetPlatformInfo(const FPlatformInfo &platform) {
    mPlatform = platform;
}

const FPlatformInfo &IBasePlayer::GetPlatformInfo() const {
    return mPlatform;
}

bool IBasePlayer::StopTimer(const FUniqueID &tid) {
    const auto timer = GetTimer(tid);
    if (timer == nullptr)
        return false;

    timer->Stop();
    return true;
}

void IBasePlayer::CleanAllTimer() {
    std::scoped_lock lock(mTimerMutex);
    for (const auto timer: mTimerMap | std::views::values) {
        delete timer;
    }
}

URepeatedTimer *IBasePlayer::GetTimer(const FUniqueID &tid) {
    std::shared_lock lock(mTimerSharedMutex);
    if (const auto it = mTimerMap.find(tid); it != mTimerMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<FUniqueID> IBasePlayer::AddTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timerID = FUniqueID::RandomGenerate(); {
        std::shared_lock lock(mTimerSharedMutex);
        while (mTimerMap.contains(timerID)) {
            timerID = FUniqueID::RandomGenerate();
        }
    } {
        std::scoped_lock lock(mTimerMutex);
        mTimerMap[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([weak = weak_from_this()](const FUniqueID &tid) mutable {
        if (const auto self = weak.lock()) {
            self->RemoveTimer(tid);
        }
    });
    return timerID;
}

bool IBasePlayer::RemoveTimer(const FUniqueID &tid) {
    std::scoped_lock lock(mTimerMutex);
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
