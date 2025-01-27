#pragma once

#include "../../SubSystem.h"
#include "../../UniqueID.h"
#include "BaseManager.h"

#include <typeindex>

class UManagerSystem final : public ISubSystem {

    std::unordered_map<std::type_index, IBaseManager *> mManagerMap;
    FUniqueID mTimerID;

public:
    explicit UManagerSystem(UGameWorld *world);
    ~UManagerSystem() override;

    GET_SYSTEM_NAME(UManagerSystem)

    void Init() override;

    template<MANAGER_TYPE T>
    T* CreateManager() {
        auto mgr = new T(this);
        mManagerMap[typeid(T)] = mgr;
        return mgr;
    }

    template<MANAGER_TYPE T>
    T *GetManager() {
        if (const auto it = mManagerMap.find(typeid(T)); it != mManagerMap.end()) {
            return dynamic_cast<T *>(it->second);
        }
        return nullptr;
    }

private:
    void OnTick(ATimePoint now);
};

#define REGISTER_MANAGER(mgr) sys->CreateManager<mgr>();
