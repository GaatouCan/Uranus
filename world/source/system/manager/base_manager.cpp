#include "../../../include/system/manager/base_manager.h"
#include "../../../include/system/manager/manager_system.h"


IBaseManager::IBaseManager(UManagerSystem *owner)
    : owner_(owner), tick_per_sec_(false) {
}

IBaseManager::~IBaseManager() {
}

UManagerSystem * IBaseManager::GetOwner() const {
    return owner_;
}

UGameWorld * IBaseManager::GetWorld() const {
    return owner_->GetWorld();
}

void IBaseManager::OnTick(ATimePoint now, ADuration interval) {
}

void IBaseManager::OnDayChange() {
}
