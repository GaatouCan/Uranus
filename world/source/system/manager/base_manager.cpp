#include "../../../include/system/manager/base_manager.h"
#include "../../../include/system/manager/manager_system.h"


IBaseManager::IBaseManager(ManagerSystem *owner)
    : mOwner(owner), tick_(false) {
}

IBaseManager::~IBaseManager() {
}

ManagerSystem * IBaseManager::GetOwner() const {
    return mOwner;
}

GameWorld * IBaseManager::GetWorld() const {
    return mOwner->GetWorld();
}

void IBaseManager::OnTick(TimePoint now) {
}

void IBaseManager::OnDayChange() {
}
