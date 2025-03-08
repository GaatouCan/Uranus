#include "../../include/scene/base_player.h"
#include "../../include/scene/base_scene.h"
#include "../../include/connection.h"
// #include "../../CrossRoute.h"

#include <utility>
#include <ranges>

IBasePlayer::IBasePlayer(const AConnectionPointer &conn)
    : owner_(nullptr),
      conn_(conn),
      pid_(std::any_cast<FPlayerID>(conn_->getContext())) {

}

IBasePlayer::~IBasePlayer() {
    for (const auto timer: timerMap_ | std::views::values) {
        delete timer;
    }
}

bool IBasePlayer::setConnection(const AConnectionPointer &conn) {
    if (std::any_cast<FPlayerID>(conn_->getContext()) != pid_) {
        return false;
    }
    conn_ = conn;
    return true;
}

AConnectionPointer IBasePlayer::getConnection() const {
    return conn_;
}

ATcpSocket &IBasePlayer::getSocket() const {
    return conn_->getSocket();
}

asio::io_context &IBasePlayer::getIOContext() const {
    const auto ctx = getSocket().get_executor().target<asio::io_context>();
    return *const_cast<asio::io_context *>(ctx);
}

UGameWorld * IBasePlayer::getWorld() const {
    return conn_->getWorld();
}

AThreadID IBasePlayer::getThreadID() const {
    return conn_->getThreadID();
}

bool IBasePlayer::isSameThread() const {
    return conn_->isSameThread();
}

int32_t IBasePlayer::getLocalID() const {
    return pid_.getLocalID();
}

int32_t IBasePlayer::getCrossID() const {
    return pid_.getCrossID();
}

const FPlayerID &IBasePlayer::getPlayerID() const {
    return pid_;
}

int64_t IBasePlayer::getFullID() const {
    return pid_.toInt64();
}

IPackage *IBasePlayer::buildPackage() const {
    return conn_->buildPackage();
}

void IBasePlayer::sendPackage(IPackage *pkg) const {
    // if (UCrossRoute::Instance().IsCrossProtocol(pkg->GetPackageID())) {
    //     UCrossRoute::Instance().Send(pkg);
    //     return;
    // }

    conn_->send(pkg);
}

void IBasePlayer::onEnterScene(IBaseScene *scene) {
    if (owner_ != nullptr) {
        // TODO
    }
    owner_ = scene;
    enterTime_ = NowTimePoint();
}

void IBasePlayer::onLeaveScene(IBaseScene *scene) {
    if (owner_ == nullptr)
        return;

    leaveTime_ = NowTimePoint();
    owner_ = nullptr;
}

bool IBasePlayer::tryLeaveScene() {
    if (owner_ == nullptr) {
        return false;
    }
    owner_->playerLeaveScene(shared_from_this());
    return true;
}

bool IBasePlayer::inScene(const int32_t id) const {
    if (owner_ == nullptr)
        return false;

    if (id == 0)
        return true;

    return owner_->getSceneID() == id;
}

int32_t IBasePlayer::getCurrentSceneID() const {
    if (owner_ == nullptr)
        return -1;
    return owner_->getSceneID();
}

IBaseScene *IBasePlayer::getCurrentScene() const {
    return owner_;
}

ATimePoint IBasePlayer::getEnterSceneTime() const {
    return enterTime_;
}

ATimePoint IBasePlayer::getLeaveSceneTime() const {
    return leaveTime_;
}

void IBasePlayer::setPlatformInfo(const FPlatformInfo &platform) {
    platform_ = platform;
}

const FPlatformInfo &IBasePlayer::getPlatformInfo() const {
    return platform_;
}

bool IBasePlayer::stopTimer(const FUniqueID &tid) {
    const auto timer = getTimer(tid);
    if (timer == nullptr)
        return false;

    timer->stop();
    return true;
}

void IBasePlayer::cleanAllTimer() {
    std::unique_lock lock(timerMutex_);
    for (const auto timer: timerMap_ | std::views::values) {
        delete timer;
    }
}

URepeatedTimer *IBasePlayer::getTimer(const FUniqueID &tid) {
    std::shared_lock lock(timerMutex_);
    if (const auto it = timerMap_.find(tid); it != timerMap_.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<FUniqueID> IBasePlayer::addTimer(URepeatedTimer *timer) {
    if (timer == nullptr)
        return std::nullopt;

    FUniqueID timer_id = FUniqueID::randomGenerate();

    {
        std::shared_lock lock(timerMutex_);
        while (timerMap_.contains(timer_id)) {
            timer_id = FUniqueID::randomGenerate();
        }
    }

    {
        std::unique_lock lock(timerMutex_);
        timerMap_[timer_id] = timer;
    }

    timer->setTimerID(timer_id).setCompleteCallback([weak = weak_from_this()](const FUniqueID &tid) mutable {
        if (const auto self = weak.lock()) {
            self->removeTimer(tid);
        }
    });
    return timer_id;
}

bool IBasePlayer::removeTimer(const FUniqueID &tid) {
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
