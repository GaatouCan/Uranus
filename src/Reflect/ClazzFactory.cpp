#include "ClazzFactory.h"
#include "Clazz.h"

UClazzFactory::UClazzFactory(UServer *server)
    : IModule(server) {
}

void UClazzFactory::Initial() {
    if (mState != EModuleState::CREATED)
        return;

    std::vector<std::string> list;

#if defined(_WIN32) || defined(_WIN64)
    for (const auto &val : list) {
        std::string path = val;
        path += ".dll";

        const auto handle = LoadLibrary(path.c_str());
        if (handle == nullptr) continue;

        auto func = reinterpret_cast<AReflectRegister>(GetProcAddress(handle, "ReflectRegister"));
        if (func != nullptr) {
            mNodeList.emplace_back(handle, func);
        } else {
            FreeLibrary(handle);
        }
    }

#else

#endif

    for (const auto &[handle, func] : mNodeList) {
        std::invoke(func, this);
    }

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
