#pragma once

#include "Common.h"

#include <atomic>
#include <concepts>


enum class BASE_API EModuleState {
    CREATED,
    INITIALIZED,
    RUNNING,
    STOPPED,
};

/**
 * The Base Class Of Server Module
 */
class BASE_API IModule {

    friend class UServer;

protected:
    explicit IModule(UServer *server);

    virtual void Initial();

    virtual void Start();
    virtual void Stop();

public:
    IModule() = delete;
    virtual ~IModule() = default;

    DISABLE_COPY_MOVE(IModule)

    virtual const char *GetModuleName() const = 0;

    [[nodiscard]] UServer *GetServer() const;

    [[nodiscard]] EModuleState GetState() const;

private:
    /** The Owner Server Pointer */
    UServer *mServer;

protected:
    /** Module Current State */
    std::atomic<EModuleState> mState;
};

template<typename T>
concept CModuleType = std::derived_from<T, IModule> && !std::is_same_v<T, IModule>;

#define DECLARE_MODULE(module) \
private: \
    friend class UServer; \
    using Super = IModule; \
public: \
    DISABLE_COPY_MOVE(module) \
private:

