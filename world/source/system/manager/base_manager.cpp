#include "../../../include/system/manager/base_manager.h"
#include "../../../include/system/manager/manager_system.h"


IBaseManager::IBaseManager(ManagerSystem *owner)
    : owner_(owner), tick_per_sec_(false) {
}

IBaseManager::~IBaseManager() {
}

ManagerSystem * IBaseManager::GetOwner() const {
    return owner_;
}

GameWorld * IBaseManager::GetWorld() const {
    return owner_->GetWorld();
}

void IBaseManager::OnTick(TimePoint now) {
}

void IBaseManager::OnDayChange() {
}
