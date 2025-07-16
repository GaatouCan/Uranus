#include "Context.h"
#include "Recycler.h"
#include "Package.h"
#include "Service/LibraryHandle.h"
#include "Service/ServiceModule.h"
#include "Service/Service.h"
#include "Event/EventParam.h"
#include "Monitor/Monitor.h"

#include <spdlog/spdlog.h>


typedef IService *(*AServiceCreator)();
typedef void (*AServiceDestroyer)(IService *);


IContext::IChannelNode::IChannelNode(IService *service)
    : mService(service) {
}

IService *IContext::IChannelNode::GetService() const {
    return mService;
}

IContext::UPackageNode::UPackageNode(IService *service)
    : IChannelNode(service),
      mPackage(nullptr) {
}

void IContext::UPackageNode::SetPackage(const shared_ptr<IPackage> &pkg) {
    mPackage = pkg;
}

void IContext::UPackageNode::Execute() {
    if (mService && mPackage) {
        mService->OnPackage(mPackage);
    }
}

IContext::UTaskNode::UTaskNode(IService *service)
    : IChannelNode(service) {
}

void IContext::UTaskNode::SetTask(const std::function<void(IService *)> &task) {
    mTask = task;
}

void IContext::UTaskNode::Execute() {
    if (mService && mTask) {
        std::invoke(mTask, mService);
    }
}

IContext::UEventNode::UEventNode(IService *service)
    : IChannelNode(service) {
}

void IContext::UEventNode::SetEventParam(const shared_ptr<IEventParam> &event) {
    mEvent = event;
}

void IContext::UEventNode::Execute() {
    if (mService && mEvent) {
        mService->OnEvent(mEvent);
    }
}

IContext::IContext()
    : mModule(nullptr),
      mService(nullptr),
      mHandle(nullptr),
      mState(EContextState::CREATED) {
}

IContext::~IContext() {
    if (mTimer) {
        mTimer->cancel();
        ForceShutdown();
    }

    if (mChannel) {
        mChannel->close();
    }
}

void IContext::SetUpModule(IModule *module) {
    if (mState != EContextState::CREATED)
        return;
    mModule = module;
}

void IContext::SetUpHandle(FLibraryHandle *handle) {
    if (mState != EContextState::CREATED)
        return;
    mHandle = handle;
}

std::string IContext::GetLibraryPath() const {
    if (mHandle == nullptr)
        return {};
    return mHandle->GetPath();
}

std::string IContext::GetServiceName() const {
    if (mState >= EContextState::INITIALIZED) {
        return mService->GetServiceName();
    }
    return "UNKNOWN";
}

IModule *IContext::GetOwner() const {
    return mModule;
}

IService *IContext::GetService() const {
    return mService;
}

void IContext::PushNode(const shared_ptr<IChannelNode> &node) {
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    if (node == nullptr)
        return;

    co_spawn(GetServer()->GetIOContext(), [self = shared_from_this(), node]() -> awaitable<void> {
        co_await self->mChannel->async_send(std::error_code{}, node);
    }, detached);
}

awaitable<void> IContext::ProcessChannel() {
    if (mState <= EContextState::INITIALIZED || mState >= EContextState::WAITING) {
        co_return;
    }

    while (mState == EContextState::IDLE || mState == EContextState::RUNNING) {
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


bool IContext::Initial(const std::shared_ptr<IPackage> &pkg) {
    if (mState != EContextState::CREATED)
        return false;

    if (mModule == nullptr || mHandle == nullptr) {
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
    mPool = GetServer()->CreatePackagePool(GetServer()->GetIOContext());
    mPool->Initial();

    // Initialize The Service With Package
    mService->SetUpContext(this);
    mService->Initial(pkg);

    // Context And Service Initialized
    mState = EContextState::INITIALIZED;
    SPDLOG_TRACE("{:<20} - Context[{:p}] Service[{}] Initial Successfully",
        __FUNCTION__, static_cast<const void *>(this), mService->GetServiceName());

    return true;
}

int IContext::Shutdown(const bool bForce, const int second, const std::function<void(IContext *)> &cb) {
    // State Maybe WAITING While If Not Force To Shut Down
    if (bForce ? mState >= EContextState::SHUTTING_DOWN : mState >= EContextState::WAITING)
        return -1;

    // If Not Force To Shut Down, Turn To Waiting Current Schedule Node Execute Complete
    if (!bForce && mState == EContextState::RUNNING) {
        mState = EContextState::WAITING;

        mTimer = make_unique<ASteadyTimer>(GetServer()->GetIOContext());
        if (cb != nullptr)
            mCallback = cb;

        // Spawn Coroutine For Waiting To Force Shut Down
        co_spawn(GetServer()->GetIOContext(), [self = shared_from_this(), second]() -> awaitable<void> {
            self->mTimer->expires_after(std::chrono::seconds(second));
            if (const auto [ec] = co_await self->mTimer->async_wait(); ec)
                co_return;

            if (self->GetState() == EContextState::WAITING) {
                self->ForceShutdown();
            }
        }, detached);

        return 0;
    }

    mState = EContextState::SHUTTING_DOWN;

    mTimer->cancel();
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

    if (mCallback) {
        std::invoke(mCallback, this);
    }

    return 1;
}

int IContext::ForceShutdown() {
    return Shutdown(true, 0, nullptr);
}


bool IContext::BootService() {
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

EContextState IContext::GetState() const {
    return mState;
}


UServer *IContext::GetServer() const {
    return mModule->GetServer();
}

void IContext::PushPackage(const std::shared_ptr<IPackage> &pkg) {
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

void IContext::PushTask(const std::function<void(IService *)> &task) {
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

void IContext::PushEvent(const std::shared_ptr<IEventParam> &event) {
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

void IContext::SendCommand(const std::string &type, const std::string &args, const std::string &comment) const {
    if (mState < EContextState::INITIALIZED)
        return;

    auto *monitor = GetModule<UMonitor>();
    if (monitor == nullptr)
        return;

    monitor->OnCommand(GetServiceName(), type, args, comment);
}


std::shared_ptr<IPackage> IContext::BuildPackage() const {
    if (mState != EContextState::IDLE || mState != EContextState::RUNNING)
        return nullptr;

    if (const auto pkg = mPool->Acquire())
        return std::dynamic_pointer_cast<IPackage>(pkg);

    return nullptr;
}

std::map<std::string, int32_t> IContext::GetServiceList() const {
    if (const auto *service = GetModule<UServiceModule>()) {
        return service->GetServiceList();
    }
    return {};
}

int32_t IContext::GetOtherServiceID(const std::string &name) const {
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

IModule *IContext::GetModuleByName(const std::string &name) const {
    return GetServer()->GetModuleByName(name);
}
