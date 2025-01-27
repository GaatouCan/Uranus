#pragma once

#include "../../reactor/Reactor.h"
#include "../../utils.h"


class BASE_API IBaseManager : public UReactor {

    class UManagerSystem *mOwner;

public:
    IBaseManager() = delete;

    explicit IBaseManager(UManagerSystem *owner);
    ~IBaseManager() override;

    DISABLE_COPY_MOVE(IBaseManager)

    virtual void Init() = 0;
    [[nodiscard]] virtual const char *GetManagerName() const = 0;

    [[nodiscard]] UManagerSystem *GetOwner() const;
    [[nodiscard]] class UGameWorld *GetWorld() const;

    virtual void OnTick(ATimePoint now);
    virtual void OnDayChange();

public:
    bool bTick;
};

template<typename T>
concept MANAGER_TYPE = std::derived_from<T, IBaseManager>;

#define GET_MANAGER_NAME(mgr) \
[[nodiscard]] constexpr const char * GetManagerName() const override { \
    return #mgr; \
} \


#define MANAGER_IMPL(mgr) \
mgr *mgr::Instance() { \
    if (const auto sys = UManagerSystem::Instance(); sys != nullptr) { \
        return sys->GetManager<mgr>(); \
    } \
    spdlog::critical("{} - Failed to get manager system.", __FUNCTION__); \
    UGameWorld::Instance().Shutdown(); \
    exit(-1); \
}

#define GET_MANAGER(mgr) GetWorld()->GetSystem<UManagerSystem>()->GetManager<mgr>();