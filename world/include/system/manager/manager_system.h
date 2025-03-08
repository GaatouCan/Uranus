#pragma once

#include "../../sub_system.h"
#include "../../unique_id.h"
#include "base_manager.h"

#include <typeindex>

class BASE_API UManagerSystem final : public ISubSystem {

    std::unordered_map<std::type_index, IBaseManager *> manager_map_;

    ASystemTimer tick_timer_;
    ATimePoint tick_point_;
    bool running_;

public:
    explicit UManagerSystem(UGameWorld *world);
    ~UManagerSystem() override;

    GET_SYSTEM_NAME(ManagerSystem)

    void init() override;

    template<CManagerType T>
    T* CreateManager() {
        auto mgr = new T(this);
        manager_map_[typeid(T)] = mgr;
        return mgr;
    }

    template<CManagerType T>
    T *GetManager() {
        if (const auto it = manager_map_.find(typeid(T)); it != manager_map_.end()) {
            return dynamic_cast<T *>(it->second);
        }
        return nullptr;
    }

private:
    void OnTick(ATimePoint now);
};

#define REGISTER_MANAGER(mgr) sys->CreateManager<mgr>();
