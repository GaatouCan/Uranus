#include "ClazzFactory.h"
#include "Clazz.h"

UClazzFactory::UClazzFactory(UServer *server)
    : IModule(server) {
}

void UClazzFactory::Initial() {
    if (mState != EModuleState::CREATED)
        return;

    mState = EModuleState::INITIALIZED;
}

UClazzFactory::~UClazzFactory() {
}

UClazz *UClazzFactory::FromName(const std::string &name) const {
    const auto iter = mClazzMap.find(name);
    return iter != mClazzMap.end() ? iter->second : nullptr;
}

void UClazzFactory::RegisterClazz(UClazz *clazz) {
    if (mState >= EModuleState::RUNNING)
        return;

    if (clazz == nullptr)
        return;

    mClazzMap[clazz->GetClazzName()] = clazz;
}
