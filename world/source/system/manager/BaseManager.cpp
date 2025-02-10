#include "../../../include/system/manager/BaseManager.h"
#include "../../../include/system/manager/ManagerSystem.h"


IBaseManager::IBaseManager(UManagerSystem *owner)
    : owner_(owner), tick_(false) {
}

IBaseManager::~IBaseManager() {
}

UManagerSystem * IBaseManager::GetOwner() const {
    return owner_;
}

UGameWorld * IBaseManager::GetWorld() const {
    return owner_->GetWorld();
}

void IBaseManager::OnTick(ATimePoint now) {
}

void IBaseManager::OnDayChange() {
}
