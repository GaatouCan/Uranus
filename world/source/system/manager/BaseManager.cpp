#include "../../../include/system/manager/BaseManager.h"
#include "../../../include/system/manager/ManagerSystem.h"


IBaseManager::IBaseManager(ManagerSystem *owner)
    : owner_(owner), tick_(false) {
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
