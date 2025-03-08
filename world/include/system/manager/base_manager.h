#pragma once

#include "../../reactor/reactor.h"
#include "../../utils.h"


class BASE_API IBaseManager : public IReactor {

    class UManagerSystem *owner_;

public:
    IBaseManager() = delete;

    explicit IBaseManager(UManagerSystem *owner);
    ~IBaseManager() override;

    DISABLE_COPY_MOVE(IBaseManager)

    virtual void init() = 0;
    [[nodiscard]] virtual const char *getManagerName() const = 0;

    [[nodiscard]] UManagerSystem *getOwner() const;
    [[nodiscard]] UGameWorld *getWorld() const;

    virtual void onTick(ATimePoint now, ADuration interval);
    virtual void onDayChange();

public:
    bool tickPerSecond_;
};

template<typename T>
concept CManagerType = std::derived_from<T, IBaseManager>;

#define GET_MANAGER_NAME(mgr) \
[[nodiscard]] constexpr const char * getManagerName() const override { \
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

#define GET_MANAGER(mgr) getWorld()->getSystem<UManagerSystem>()->getManager<mgr>()