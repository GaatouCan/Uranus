#pragma once

#include "ConcurrentDeque.h"
#include "Server.h"
#include "utils.h"

#include <memory>


class IService;
class IModule;
class IEventParam;
class UServer;
class IPackage;
class FLibraryHandle;


// ~Begin Define Schedule Node

/**
 * The Schedulable Node For Context To Schedule
 */
class BASE_API IScheduleNode {

protected:
    IService *const mService;

public:
    IScheduleNode() = delete;

    explicit IScheduleNode(IService *service);
    virtual ~IScheduleNode() = default;

    DISABLE_COPY_MOVE(IScheduleNode)

    [[nodiscard]] IService *GetService() const;

    virtual void Execute() = 0;
};

/**
 * The Wrapper Of Package,
 * While Service Received Package
 */
class BASE_API UPackageNode final : public IScheduleNode {

    std::shared_ptr<IPackage> mPackage;

public:
    explicit UPackageNode(IService *service);
    ~UPackageNode() override = default;

    void SetPackage(const std::shared_ptr<IPackage> &pkg);
    void Execute() override;
};

/**
 * The Wrapper Of Task,
 * While The Service Received The Task
 */
class BASE_API UTaskNode final : public IScheduleNode {

    std::function<void(IService *)> mTask;

public:
    explicit UTaskNode(IService *service);
    ~UTaskNode() override = default;

    void SetTask(const std::function<void(IService *)> &task);
    void Execute() override;
};

/**
 * The Wrapper Of Event,
 * While The Service Received Event Parameter
 */
class BASE_API UEventNode final : public IScheduleNode {

    std::shared_ptr<IEventParam> mEvent;

public:
    explicit UEventNode(IService *service);
    ~UEventNode() override = default;

    void SetEventParam(const std::shared_ptr<IEventParam> &event);
    void Execute() override;
};

// ~End Define Schedule Node

enum class BASE_API EContextState {
    CREATED,
    INITIALIZING,
    INITIALIZED,
    IDLE,
    RUNNING,
    WAITING,
    SHUTTING_DOWN,
    STOPPED
};

/**
 * Context For Interaction Between Service And The Server
 */
class BASE_API IContext : public std::enable_shared_from_this<IContext> {

    using AScheduleQueue = TConcurrentDeque<std::unique_ptr<IScheduleNode>>;

    /** The Owner Module */
    IModule *mModule;

    /** The Owned Service */
    IService *mService;

    /** Loaded Library With Creator And Destroyer Of Service */
    FLibraryHandle *mHandle;

    /** Internal Package Pool */
    std::shared_ptr<IRecycler> mPool;

    /** Internal Schedule Queue */
    std::unique_ptr<AScheduleQueue> mQueue;

    /** When Timeout, Force Shut Down This Context */
    std::shared_ptr<ASystemTimer> mShutdownTimer;

    /** Called While This Context Stopped */
    std::function<void(IContext *)> mShutdownCallback;

protected:
    /** Current Context State */
    std::atomic<EContextState> mState;

public:
    IContext();
    virtual ~IContext();

    void SetUpModule(IModule *module);
    void SetUpHandle(FLibraryHandle *handle);

    /** Return The Path String Of The Dynamic Library File That The Service Defined */
    std::string GetLibraryPath() const;

    [[nodiscard]] std::string GetServiceName() const;
    [[nodiscard]] virtual int32_t GetServiceID() const = 0;

    /** Create The Service Ant Initial It, And Other Resource It Needs */
    virtual bool Initial(const std::shared_ptr<IPackage> &pkg);

    /** Shutdown And Delete The Service, Release The Resource */
    virtual int Shutdown(bool bFource, int second, const std::function<void(IContext *)> &cb);

    /** Equal Shutdown(true, 5, nullptr) */
    int ForceShutdown();

    bool BootService();

    [[nodiscard]] EContextState GetState() const;
    [[nodiscard]] UServer *GetServer() const;

    void PushPackage(const std::shared_ptr<IPackage> &pkg);
    void PushTask(const std::function<void(IService *)> &task);
    void PushEvent(const std::shared_ptr<IEventParam> &event);

    void SendCommand(const std::string &type, const std::string &args, const std::string &comment = "") const;

    /** Acquire One Package From Internal Package Pool */
    std::shared_ptr<IPackage> BuildPackage() const;

    [[nodiscard]] std::map<std::string, int32_t> GetServiceList() const;
    [[nodiscard]] int32_t GetOtherServiceID(const std::string &name) const;

    template<CModuleType Module>
    Module *GetModule() const;
    [[nodiscard]] IModule *GetModuleByName(const std::string &name) const;

protected:
    IModule *GetOwner() const;
    IService *GetService() const;

private:
    void PushNode(std::unique_ptr<IScheduleNode> &&node);
    void DoSchedule();
};


template<CModuleType Module>
inline Module *IContext::GetModule() const {
    return GetServer()->GetModule<Module>();
}
