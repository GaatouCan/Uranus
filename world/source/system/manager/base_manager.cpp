#include "../../../include/system/manager/base_manager.h"
#include "../../../include/system/manager/manager_system.h"


IBaseManager::IBaseManager(UManagerSystem *owner)
    : owner_(owner), tickPerSecond_(false) {
}

IBaseManager::~IBaseManager() {
}

UManagerSystem * IBaseManager::getOwner() const {
    return owner_;
}

UGameWorld * IBaseManager::getWorld() const {
    return owner_->getWorld();
}

void IBaseManager::onTick(ATimePoint now, ADuration interval) {
}

void IBaseManager::onDayChange() {
}
