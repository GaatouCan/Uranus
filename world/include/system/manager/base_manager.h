#pragma once

#include "../../reactor/reactor.h"
#include "../../utils.h"


class BASE_API IBaseManager : public IReactor {

    class ManagerSystem *owner_;

public:
    IBaseManager() = delete;

    explicit IBaseManager(ManagerSystem *owner);
    ~IBaseManager() override;

    DISABLE_COPY_MOVE(IBaseManager)

    virtual void Init() = 0;
    [[nodiscard]] virtual const char *GetManagerName() const = 0;

    [[nodiscard]] ManagerSystem *GetOwner() const;
    [[nodiscard]] class GameWorld *GetWorld() const;

    virtual void OnTick(TimePoint now);
    virtual void OnDayChange();

public:
    bool tick_per_sec_;
};

template<typename T>
concept ManagerType = std::derived_from<T, IBaseManager>;

#define GET_MANAGER_NAME(mgr) \
[[nodiscard]] constexpr const char * GetManagerName() const override { \
    return #mgr; \
} \


#define MANAGER_IMPL(mgr) \
mgr *mgr::Instance() { \
    if (const auto sys = ManagerSystem::Instance(); sys != nullptr) { \
        return sys->GetManager<mgr>(); \
    } \
    spdlog::critical("{} - Failed to get manager system.", __FUNCTION__); \
    UGameWorld::Instance().Shutdown(); \
    exit(-1); \
}

#define GET_MANAGER(mgr) GetWorld()->GetSystem<ManagerSystem>()->GetManager<mgr>()