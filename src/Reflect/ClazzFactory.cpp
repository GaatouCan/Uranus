#include "ClazzFactory.h"
#include "Clazz.h"

UClazzFactory::UClazzFactory() {
}

void UClazzFactory::Initial() {
    if (state_ != EModuleState::CREATED)
        return;

    state_ = EModuleState::INITIALIZED;
}

UClazzFactory::~UClazzFactory() {
}

UClazz *UClazzFactory::FromName(const std::string &name) const {
    std::shared_lock lock(mMutex);
    const auto iter = mClazzMap.find(name);
    return iter != mClazzMap.end() ? iter->second : nullptr;
}

void UClazzFactory::RemoveClazz(const std::string &name) {
    std::unique_lock lock(mMutex);
    mClazzMap.erase(name);
}

void UClazzFactory::RegisterClazz(UClazz *clazz) {
    if (state_ >= EModuleState::RUNNING)
        return;

    if (clazz == nullptr)
        return;

    std::unique_lock lock(mMutex);
    mClazzMap[clazz->GetClazzName()] = clazz;
}
