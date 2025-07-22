#pragma once

#include "Server.h"
#include "Types.h"

#include <memory>


class IServiceBase;
class IModuleBase;
class IEventInterface;
class UServer;
class IPackageInterface;
class FLibraryHandle;


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
class BASE_API IContextBase : public std::enable_shared_from_this<IContextBase> {

    /**
 * The Schedulable Node For Context To Schedule
 */
    class BASE_API INodeBase {

    protected:
        IServiceBase *const mService;

    public:
        INodeBase() = delete;

        explicit INodeBase(IServiceBase *service);
        virtual ~INodeBase() = default;

        DISABLE_COPY_MOVE(INodeBase)

        [[nodiscard]] IServiceBase *GetService() const;

        virtual void Execute() = 0;
    };

    /**
     * The Wrapper Of Package,
     * While Service Received Package
     */
    class BASE_API UPackageNode final : public INodeBase {

        shared_ptr<IPackageInterface> mPackage;

    public:
        explicit UPackageNode(IServiceBase *service);
        ~UPackageNode() override = default;

        void SetPackage(const shared_ptr<IPackageInterface> &pkg);
        void Execute() override;
    };

    /**
     * The Wrapper Of Task,
     * While The Service Received The Task
     */
    class BASE_API UTaskNode final : public INodeBase {

        std::function<void(IServiceBase *)> mTask;

    public:
        explicit UTaskNode(IServiceBase *service);
        ~UTaskNode() override = default;

        void SetTask(const std::function<void(IServiceBase *)> &task);
        void Execute() override;
    };

    /**
     * The Wrapper Of Event,
     * While The Service Received Event Parameter
     */
    class BASE_API UEventNode final : public INodeBase {

        shared_ptr<IEventInterface> mEvent;

    public:
        explicit UEventNode(IServiceBase *service);
        ~UEventNode() override = default;

        void SetEventParam(const shared_ptr<IEventInterface> &event);
        void Execute() override;
    };

    using AContextChannel = DefaultToken::as_default_on_t<asio::experimental::concurrent_channel<void(std::error_code, shared_ptr<INodeBase>)>>;

    /** The Owner Module */
    IModuleBase *mOwner;

    /** The Owned Service */
    IServiceBase *mService;

    /** Loaded Library With Creator And Destroyer Of Service */
    FLibraryHandle *mHandle;

    /** Internal Package Pool */
    shared_ptr<IRecyclerBase> mPackagePool;

    /** Internal Node Channel */
    unique_ptr<AContextChannel> mChannel;

    /** When Timeout, Force Shut Down This Context */
    unique_ptr<ASteadyTimer> mShutdownTimer;

    /** Invoked While This Context Stopped */
    std::function<void(IContextBase *)> mShutdownCallback;

protected:
    /** Current Context State */
    std::atomic<EContextState> mState;

public:
    IContextBase();
    virtual ~IContextBase();

    DISABLE_COPY_MOVE(IContextBase)

    void SetUpModule(IModuleBase *module);
    void SetUpHandle(FLibraryHandle *handle);

    /** Return The Path String Of The Dynamic Library File That The Service Defined */
    std::string GetLibraryPath() const;

    [[nodiscard]] std::string GetServiceName() const;
    [[nodiscard]] virtual int32_t GetServiceID() const = 0;

    /** Create The Service Ant Initial It, And Other Resource It Needs */
    virtual bool Initial(const shared_ptr<IPackageInterface> &pkg);

    /** Shutdown And Delete The Service, Release The Resource */
    virtual int Shutdown(bool bForce, int second, const std::function<void(IContextBase *)> &cb);

    /** Equal Shutdown(true, 5, nullptr) */
    int ForceShutdown();

    bool BootService();

    [[nodiscard]] EContextState GetState() const;
    [[nodiscard]] UServer *GetServer() const;

    void PushPackage(const shared_ptr<IPackageInterface> &pkg);
    void PushTask(const std::function<void(IServiceBase *)> &task);
    void PushEvent(const shared_ptr<IEventInterface> &event);

    void SendCommand(const std::string &type, const std::string &args, const std::string &comment = "") const;

    /** Acquire One Package From Internal Package Pool */
    shared_ptr<IPackageInterface> BuildPackage() const;

    [[nodiscard]] std::map<std::string, int32_t> GetServiceList() const;
    [[nodiscard]] int32_t GetOtherServiceID(const std::string &name) const;

    template<CModuleType Module>
    Module *GetModule() const;
    [[nodiscard]] IModuleBase *GetModule(const std::string &name) const;

protected:
    IModuleBase *GetOwner() const;
    IServiceBase *GetService() const;

private:
    void PushNode(const std::shared_ptr<INodeBase> &node);
    awaitable<void> ProcessChannel();
};


template<CModuleType Module>
inline Module *IContextBase::GetModule() const {
    return GetServer()->GetModule<Module>();
}
