#pragma once

#include "../../sub_system.h"
#include "../../unique_id.h"
#include "base_manager.h"

#include <typeindex>

class ManagerSystem final : public ISubSystem {

    std::unordered_map<std::type_index, IBaseManager *> mManagerMap;

    SystemTimer mTickTimer;
    bool bRunning;

public:
    explicit ManagerSystem(GameWorld *world);
    ~ManagerSystem() override;

    GET_SYSTEM_NAME(ManagerSystem)

    void Init() override;

    template<ManagerType T>
    T* CreateManager() {
        auto mgr = new T(this);
        mManagerMap[typeid(T)] = mgr;
        return mgr;
    }

    template<ManagerType T>
    T *GetManager() {
        if (const auto it = mManagerMap.find(typeid(T)); it != mManagerMap.end()) {
            return dynamic_cast<T *>(it->second);
        }
        return nullptr;
    }

private:
    void OnTick(TimePoint now);
};

#define REGISTER_MANAGER(mgr) sys->CreateManager<mgr>();
