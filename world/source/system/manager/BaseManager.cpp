#include "../../../include/system/manager/BaseManager.h"
#include "../../../include/system/manager/ManagerSystem.h"


IBaseManager::IBaseManager(UManagerSystem *owner)
    : mOwner(owner), bTick(false) {
}

IBaseManager::~IBaseManager() {
}

UManagerSystem * IBaseManager::GetOwner() const {
    return mOwner;
}

UGameWorld * IBaseManager::GetWorld() const {
    return mOwner->GetWorld();
}

void IBaseManager::OnTick(ATimePoint now) {
}

void IBaseManager::OnDayChange() {
}
