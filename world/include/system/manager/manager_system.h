#pragma once

#include "../../SubSystem.h"
#include "../../UniqueID.h"
#include "BaseManager.h"

#include <typeindex>

class ManagerSystem final : public ISubSystem {

    std::unordered_map<std::type_index, IBaseManager *> manager_map_;
    UniqueID timer_id_;

public:
    explicit ManagerSystem(GameWorld *world);
    ~ManagerSystem() override;

    GET_SYSTEM_NAME(UManagerSystem)

    void Init() override;

    template<ManagerType T>
    T* CreateManager() {
        auto mgr = new T(this);
        manager_map_[typeid(T)] = mgr;
        return mgr;
    }

    template<ManagerType T>
    T *GetManager() {
        if (const auto it = manager_map_.find(typeid(T)); it != manager_map_.end()) {
            return dynamic_cast<T *>(it->second);
        }
        return nullptr;
    }

private:
    void OnTick(TimePoint now);
};

#define REGISTER_MANAGER(mgr) sys->CreateManager<mgr>();
