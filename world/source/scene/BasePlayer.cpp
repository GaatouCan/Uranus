#include "../../include/scene/BasePlayer.h"
#include "scene/AbstractScene.h"
// #include "../../CrossRoute.h"

#include <utility>
#include <ranges>

IBasePlayer::IBasePlayer(AConnectionPointer conn)
    : owner_(nullptr),
      conn_(std::move(conn)),
      pid_(std::any_cast<FPlayerID>(conn_->GetContext())) {

}

IBasePlayer::~IBasePlayer() {
    for (const auto timer: timerMap_ | std::views::values) {
        delete timer;
    }
}

bool IBasePlayer::SetConnection(AConnectionPointer conn) {
    if (std::any_cast<FPlayerID>(conn_->GetContext()) != pid_) {
        return false;
    }
    conn_ = std::move(conn);
    return true;
}

AConnectionPointer IBasePlayer::GetConnection() const {
    return conn_;
}

ATcpSocket &IBasePlayer::GetSocket() const {
    return conn_->GetSocket();
}

asio::io_context &IBasePlayer::GetIOContext() const {
    const auto ctx = GetSocket().get_executor().target<asio::io_context>();
    return *const_cast<asio::io_context *>(ctx);
}

UGameWorld * IBasePlayer::GetWorld() const {
    return conn_->GetWorld();
}

AThreadID IBasePlayer::GetThreadID() const {
    return conn_->GetThreadID();
}

bool IBasePlayer::IsSameThread() const {
    return conn_->IsSameThread();
}

int32_t IBasePlayer::GetLocalID() const {
    return pid_.GetLocalID();
}

int32_t IBasePlayer::GetCrossID() const {
    return pid_.GetCrossID();
}

const FPlayerID &IBasePlayer::GetPlayerID() const {
    return pid_;
}

int64_t IBasePlayer::GetFullID() const {
    return pid_.ToInt64();
}

IPackage *IBasePlayer::BuildPackage() const {
    return conn_->BuildPackage();
}

void IBasePlayer::SendPackage(IPackage *pkg) const {
    // if (UCrossRoute::Instance().IsCrossProtocol(pkg->GetPackageID())) {
    //     UCrossRoute::Instance().Send(pkg);
    //     return;
    // }

    conn_->Send(pkg);
}

void IBasePlayer::OnEnterScene(IAbstractScene *scene) {
    if (owner_ != nullptr) {
        // TODO
    }
    owner_ = scene;
    enterTime_ = NowTimePoint();
}

void IBasePlayer::OnLeaveScene(IAbstractScene *scene) {
    if (owner_ == nullptr)
        return;

    leaveTime_ = NowTimePoint();
    owner_ = nullptr;
}

bool IBasePlayer::TryLeaveScene() {
    if (owner_ == nullptr) {
        return false;
    }
    owner_->PlayerLeaveScene(shared_from_this());
    return true;
}

bool IBasePlayer::IsInScene(const int32_t id) const {
    if (owner_ == nullptr)
        return false;

    if (id == 0)
        return true;

    return owner_->GetSceneID() == id;
}

int32_t IBasePlayer::GetCurrentSceneID() const {
    if (owner_ == nullptr)
        return -1;
    return owner_->GetSceneID();
}

IAbstractScene *IBasePlayer::GetCurrentScene() const {
    return owner_;
}

ATimePoint IBasePlayer::GetEnterSceneTime() const {
    return enterTime_;
}

ATimePoint IBasePlayer::GetLeaveSceneTime() const {
    return leaveTime_;
}

void IBasePlayer::SetPlatformInfo(const FPlatformInfo &platform) {
    platform_ = platform;
}

const FPlatformInfo &IBasePlayer::GetPlatformInfo() const {
    return platform_;
}

bool IBasePlayer::StopTimer(const FUniqueID &tid) {
    const auto timer = GetTimer(tid);
    if (timer == nullptr)
        return false;

    timer->Stop();
    return true;
}

void IBasePlayer::CleanAllTimer() {
    std::unique_lock lock(timerMutex_);
    for (const auto timer: timerMap_ | std::views::values) {
        delete timer;
    }
}

URepeatedTimer *IBasePlayer::GetTimer(const FUniqueID &tid) {
    std::shared_lock lock(timerMutex_);
    if (const auto it = timerMap_.find(tid); it != timerMap_.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<FUniqueID> IBasePlayer::AddTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timerID = FUniqueID::RandomGenerate();

    {
        std::shared_lock lock(timerMutex_);
        while (timerMap_.contains(timerID)) {
            timerID = FUniqueID::RandomGenerate();
        }
    }

    {
        std::unique_lock lock(timerMutex_);
        timerMap_[timerID] = timer;
    }

    timer->SetTimerID(timerID).SetCompleteCallback([weak = weak_from_this()](const FUniqueID &tid) mutable {
        if (const auto self = weak.lock()) {
            self->RemoveTimer(tid);
        }
    });
    return timerID;
}

bool IBasePlayer::RemoveTimer(const FUniqueID &tid) {
    std::unique_lock lock(timerMutex_);
    if (const auto it = timerMap_.find(tid); it != timerMap_.end()) {
        const auto timer = it->second;
        timerMap_.erase(it);

        if (timer != nullptr) {
            delete timer;
            return true;
        }
    }
    return false;
}
