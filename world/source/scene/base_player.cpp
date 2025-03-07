#include "../../include/scene/base_player.h"
#include "../../include/scene/base_scene.h"
#include "../../include/connection.h"
// #include "../../CrossRoute.h"

#include <utility>
#include <ranges>

IBasePlayer::IBasePlayer(const AConnectionPointer &conn)
    : owner_(nullptr),
      conn_(conn),
      pid_(std::any_cast<FPlayerID>(conn_->GetContext())) {

}

IBasePlayer::~IBasePlayer() {
    for (const auto timer: timer_map_ | std::views::values) {
        delete timer;
    }
}

bool IBasePlayer::SetConnection(const AConnectionPointer &conn) {
    if (std::any_cast<FPlayerID>(conn_->GetContext()) != pid_) {
        return false;
    }
    conn_ = conn;
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

void IBasePlayer::OnEnterScene(IBaseScene *scene) {
    if (owner_ != nullptr) {
        // TODO
    }
    owner_ = scene;
    enter_time_ = NowTimePoint();
}

void IBasePlayer::OnLeaveScene(IBaseScene *scene) {
    if (owner_ == nullptr)
        return;

    leave_time_ = NowTimePoint();
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

IBaseScene *IBasePlayer::GetCurrentScene() const {
    return owner_;
}

ATimePoint IBasePlayer::GetEnterSceneTime() const {
    return enter_time_;
}

ATimePoint IBasePlayer::GetLeaveSceneTime() const {
    return leave_time_;
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
    std::unique_lock lock(timer_mtx_);
    for (const auto timer: timer_map_ | std::views::values) {
        delete timer;
    }
}

URepeatedTimer *IBasePlayer::GetTimer(const FUniqueID &tid) {
    std::shared_lock lock(timer_mtx_);
    if (const auto it = timer_map_.find(tid); it != timer_map_.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<FUniqueID> IBasePlayer::AddTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timer_id = FUniqueID::RandomGenerate();

    {
        std::shared_lock lock(timer_mtx_);
        while (timer_map_.contains(timer_id)) {
            timer_id = FUniqueID::RandomGenerate();
        }
    }

    {
        std::unique_lock lock(timer_mtx_);
        timer_map_[timer_id] = timer;
    }

    timer->SetTimerID(timer_id).SetCompleteCallback([weak = weak_from_this()](const FUniqueID &tid) mutable {
        if (const auto self = weak.lock()) {
            self->RemoveTimer(tid);
        }
    });
    return timer_id;
}

bool IBasePlayer::RemoveTimer(const FUniqueID &tid) {
    std::unique_lock lock(timer_mtx_);
    if (const auto it = timer_map_.find(tid); it != timer_map_.end()) {
        const auto timer = it->second;
        timer_map_.erase(it);

        if (timer != nullptr) {
            delete timer;
            return true;
        }
    }
    return false;
}
