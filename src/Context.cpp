#include "Context.h"
#include "Recycler.h"
#include "Package.h"
#include "Service/LibraryHandle.h"
#include "Service/ServiceModule.h"
#include "Service/Service.h"
#include "Event/Event.h"
#include "Monitor/Monitor.h"

#include <spdlog/spdlog.h>


typedef IServiceBase *(*AServiceCreator)();
typedef void (*AServiceDestroyer)(IServiceBase *);


IContextBase::INodeBase::INodeBase(IServiceBase *service)
    : mService(service) {
}

IServiceBase *IContextBase::INodeBase::GetService() const {
    return mService;
}

IContextBase::UPackageNode::UPackageNode(IServiceBase *service)
    : INodeBase(service),
      mPackage(nullptr) {
}

void IContextBase::UPackageNode::SetPackage(const shared_ptr<IPackageInterface> &pkg) {
    mPackage = pkg;
}

void IContextBase::UPackageNode::Execute() {
    if (mService && mPackage) {
        mService->OnPackage(mPackage);
    }
}

IContextBase::UTaskNode::UTaskNode(IServiceBase *service)
    : INodeBase(service) {
}

void IContextBase::UTaskNode::SetTask(const std::function<void(IServiceBase *)> &task) {
    mTask = task;
}

void IContextBase::UTaskNode::Execute() {
    if (mService && mTask) {
        std::invoke(mTask, mService);
    }
}

IContextBase::UEventNode::UEventNode(IServiceBase *service)
    : INodeBase(service) {
}

void IContextBase::UEventNode::SetEventParam(const shared_ptr<IEventInterface> &event) {
    mEvent = event;
}

void IContextBase::UEventNode::Execute() {
    if (mService && mEvent) {
        mService->OnEvent(mEvent);
    }
}

IContextBase::IContextBase()
    : mOwner(nullptr),
      mService(nullptr),
      mHandle(nullptr),
      mState(EContextState::CREATED) {
}

IContextBase::~IContextBase() {
    if (mShutdownTimer) {
        mShutdownTimer->cancel();
        ForceShutdown();
    }

    if (mChannel) {
        mChannel->close();
    }
}

void IContextBase::SetUpModule(IModuleBase *module) {
    if (mState != EContextState::CREATED)
        return;
    mOwner = module;
}

void IContextBase::SetUpHandle(FLibraryHandle *handle) {
    if (mState != EContextState::CREATED)
        return;
    mHandle = handle;
}

std::string IContextBase::GetLibraryPath() const {
    if (mHandle == nullptr)
        return {};
    return mHandle->GetPath();
}

std::string IContextBase::GetServiceName() const {
    if (mState >= EContextState::INITIALIZED) {
        return mService->GetServiceName();
    }
    return "UNKNOWN";
}

IModuleBase *IContextBase::GetOwner() const {
    return mOwner;
}

IServiceBase *IContextBase::GetService() const {
    return mService;
}

void IContextBase::PushNode(const shared_ptr<INodeBase> &node) {
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    if (node == nullptr)
        return;

    co_spawn(GetServer()->GetIOContext(), [self = shared_from_this(), node]() -> awaitable<void> {
        co_await self->mChannel->async_send(std::error_code{}, node);
    }, detached);
}

awaitable<void> IContextBase::ProcessChannel() {
    if (mState <= EContextState::INITIALIZED || mState >= EContextState::WAITING)
        co_return;

    while (mChannel->is_open()) {
        const auto [ec, node] = co_await mChannel->async_receive();
        if (ec)
            co_return;

        if (node == nullptr)
            continue;

        if (mState >= EContextState::WAITING)
            co_return;

        mState = EContextState::RUNNING;
        try {
            node->Execute();
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
        }

        if (mState >= EContextState::WAITING) {
            ForceShutdown();
            co_return;
        }

        mState = EContextState::IDLE;
    }
}


bool IContextBase::Initial(const std::shared_ptr<IPackageInterface> &pkg) {
    if (mState != EContextState::CREATED)
        return false;

    if (mOwner == nullptr || mHandle == nullptr) {
        SPDLOG_ERROR("{:<20} - Owner Module Or Library Node Is Null", __FUNCTION__);
        return false;
    }

    // Start To Create Service
    mState = EContextState::INITIALIZING;

    auto creator = mHandle->GetCreatorT<AServiceCreator>();
    if (creator == nullptr) {
        SPDLOG_ERROR("{:<20} - Can't Load Creator, Path[{}]", __FUNCTION__, mHandle->GetPath());
        mState = EContextState::CREATED;
        return false;
    }

    mService = std::invoke(creator);
    if (mService == nullptr) {
        SPDLOG_ERROR("{:<20} - Can't Create Service, Path[{}]", __FUNCTION__, mHandle->GetPath());
        mState = EContextState::CREATED;
        return false;
    }

    // Create Node Queue For Schedule
    mChannel = make_unique<AContextChannel>(GetServer()->GetIOContext(), 1024);

    // Create Package Pool For Data Exchange
    mPackagePool = GetServer()->CreatePackagePool(GetServer()->GetIOContext());
    mPackagePool->Initial();

    // Initialize The Service With Package
    mService->SetUpContext(this);
    mService->Initial(pkg);

    // Context And Service Initialized
    mState = EContextState::INITIALIZED;
    SPDLOG_TRACE("{:<20} - Context[{:p}] Service[{}] Initial Successfully",
        __FUNCTION__, static_cast<const void *>(this), mService->GetServiceName());

    return true;
}

int IContextBase::Shutdown(const bool bForce, const int second, const std::function<void(IContextBase *)> &cb) {
    // State Maybe WAITING While If Not Force To Shut Down
    if (bForce ? mState >= EContextState::SHUTTING_DOWN : mState >= EContextState::WAITING)
        return -1;

    // If Not Force To Shut Down, Turn To Waiting Current Schedule Node Execute Complete
    if (!bForce && mState == EContextState::RUNNING) {
        mState = EContextState::WAITING;

        mShutdownTimer = make_unique<ASteadyTimer>(GetServer()->GetIOContext());
        if (cb != nullptr)
            mShutdownCallback = cb;

        // Spawn Coroutine For Waiting To Force Shut Down
        co_spawn(GetServer()->GetIOContext(), [self = shared_from_this(), second]() -> awaitable<void> {
            self->mShutdownTimer->expires_after(std::chrono::seconds(second));
            if (const auto [ec] = co_await self->mShutdownTimer->async_wait(); ec)
                co_return;

            if (self->GetState() == EContextState::WAITING) {
                self->ForceShutdown();
            }
        }, detached);

        return 0;
    }

    mState = EContextState::SHUTTING_DOWN;

    mShutdownTimer->cancel();
    mChannel->close();

    const std::string name = GetServiceName();

    if (mService && mService->GetState() != EServiceState::TERMINATED) {
        mService->Stop();
    }

    if (mHandle == nullptr) {
        SPDLOG_ERROR("{:<20} - Library Node Is Null", __FUNCTION__);
        mState = EContextState::STOPPED;
        return -2;
    }

    auto destroyer = mHandle->GetDestroyerT<AServiceDestroyer>();
    if (destroyer == nullptr) {
        SPDLOG_ERROR("{:<20} - Can't Load Destroyer, Path[{}]", __FUNCTION__, mHandle->GetPath());
        mState = EContextState::STOPPED;
        return -3;
    }

    std::invoke(destroyer, mService);

    mService = nullptr;
    mHandle = nullptr;

    mState = EContextState::STOPPED;
    SPDLOG_TRACE("{:<20} - Context[{:p}] Service[{}] Shut Down Successfully",
        __FUNCTION__, static_cast<void *>(this), name);

    if (mShutdownCallback) {
        std::invoke(mShutdownCallback, this);
    }

    return 1;
}

int IContextBase::ForceShutdown() {
    return Shutdown(true, 0, nullptr);
}


bool IContextBase::BootService() {
    if (mState != EContextState::INITIALIZED || mService == nullptr)
        return false;

    mState = EContextState::IDLE;

    if (const auto res = mService->Start(); !res) {
        SPDLOG_ERROR("{:<20} - Context[{:p}], Service[{} - {}] Failed To Boot.",
            __FUNCTION__, static_cast<const void *>(this), GetServiceID(), GetServiceName());

        mState = EContextState::INITIALIZED;
        return false;
    }

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{} - {}] Started.",
        __FUNCTION__, static_cast<const void *>(this), GetServiceID(), GetServiceName());

    co_spawn(GetServer()->GetIOContext(), [self = shared_from_this()] -> awaitable<void> {
        co_await self->ProcessChannel();
    }, detached);

    return true;
}

EContextState IContextBase::GetState() const {
    return mState;
}


UServer *IContextBase::GetServer() const {
    return mOwner->GetServer();
}

void IContextBase::PushPackage(const std::shared_ptr<IPackageInterface> &pkg) {
    // Could Receive Package After Initialized And Before Waiting
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    if (pkg == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}] - Package From {}",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName(), pkg->GetSource());

    const auto node = make_shared<UPackageNode>(mService);
    node->SetPackage(pkg);

    PushNode(node);
}

void IContextBase::PushTask(const std::function<void(IServiceBase *)> &task) {
    // As Same As ::PushPackage()
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    if (task == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}]",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName());

    const auto node = make_shared<UTaskNode>(mService);
    node->SetTask(task);

    PushNode(node);
}

void IContextBase::PushEvent(const std::shared_ptr<IEventInterface> &event) {
    // As Same As ::PushPackage()
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    if (event == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}] - Event Type {}",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName(), event->GetEventType());

    const auto node = make_shared<UEventNode>(mService);
    node->SetEventParam(event);

    PushNode(node);
}

void IContextBase::SendCommand(const std::string &type, const std::string &args, const std::string &comment) const {
    if (mState < EContextState::INITIALIZED)
        return;

    auto *monitor = GetModule<UMonitor>();
    if (monitor == nullptr)
        return;

    monitor->OnCommand(GetServiceName(), type, args, comment);
}


std::shared_ptr<IPackageInterface> IContextBase::BuildPackage() const {
    if (mState != EContextState::IDLE || mState != EContextState::RUNNING)
        return nullptr;

    if (const auto elem = mPackagePool->Acquire())
        return std::dynamic_pointer_cast<IPackageInterface>(elem);

    return nullptr;
}

std::map<std::string, int32_t> IContextBase::GetServiceList() const {
    if (const auto *service = GetModule<UServiceModule>()) {
        return service->GetServiceList();
    }
    return {};
}

int32_t IContextBase::GetOtherServiceID(const std::string &name) const {
    if (mState < EContextState::INITIALIZED)
        return -10;

    // Do Not Find Self
    if (name.empty() || name == GetServiceName())
        return -11;

    if (const auto *service = GetModule<UServiceModule>()) {
        if (const auto sid = service->GetServiceID(name); sid != GetServiceID())
            return sid;
        return -14;
    }

    return -13;
}

IModuleBase *IContextBase::GetModule(const std::string &name) const {
    return GetServer()->GetModule(name);
}
