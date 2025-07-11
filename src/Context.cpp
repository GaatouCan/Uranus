#include "Context.h"
#include "Recycler.h"
#include "Package.h"
#include "Service/ServiceHandle.h"
#include "Service/ServiceModule.h"
#include "Service/Service.h"
#include "Event/EventParam.h"
#include "Monitor/Monitor.h"

#include <spdlog/spdlog.h>


typedef IService *(*AServiceCreator)();
typedef void (*AServiceDestroyer)(IService *);


IScheduleNode::IScheduleNode(IService *service)
    : mService(service) {
}

IService *IScheduleNode::GetService() const {
    return mService;
}

UPackageNode::UPackageNode(IService *service)
    : IScheduleNode(service),
      mPackage(nullptr) {
}

void UPackageNode::SetPackage(const std::shared_ptr<IPackage> &pkg) {
    mPackage = pkg;
}

void UPackageNode::Execute() {
    if (mService && mPackage) {
        mService->OnPackage(mPackage);
    }
}

UTaskNode::UTaskNode(IService *service)
    : IScheduleNode(service) {
}

void UTaskNode::SetTask(const std::function<void(IService *)> &task) {
    mTask = task;
}

void UTaskNode::Execute() {
    if (mService && mTask) {
        std::invoke(mTask, mService);
    }
}

UEventNode::UEventNode(IService *service)
    : IScheduleNode(service) {
}

void UEventNode::SetEventParam(const std::shared_ptr<IEventParam> &event) {
    mEvent = event;
}

void UEventNode::Execute() {
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
    if (mShutdownTimer) {
        mShutdownTimer->cancel();
        ForceShutdown();
    }

    if (mQueue != nullptr && !mQueue->IsEmpty())
        mQueue->Clear();
}

void IContext::SetUpModule(IModule *module) {
    if (mState != EContextState::CREATED)
        return;
    mModule = module;
}

void IContext::SetUpHandle(FServiceHandle *handle) {
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

void IContext::PushNode(std::unique_ptr<IScheduleNode> &&node) {
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    const auto bEmpty = mQueue->IsEmpty();
    mQueue->PushBack(std::move(node));

    if (bEmpty && mState == EContextState::IDLE) {
        mState = EContextState::RUNNING;
        co_spawn(GetServer()->GetIOContext(), [weak = weak_from_this()]() -> awaitable<void> {
            if (const auto self = weak.lock()) {
                self->DoSchedule();
            }
            co_return;
        }, detached);
    }
}

void IContext::DoSchedule() {

    auto shutdown = [this] {
        if (mShutdownTimer) {
            mShutdownTimer->cancel();
        }
        ForceShutdown();
    };

    // If Already Call ::Shutdown() To Wait Before Schedule
    if (mState == EContextState::WAITING) {
        shutdown();
        return;
    }

    if (mState != EContextState::RUNNING && mState != EContextState::IDLE)
        return;

    if (mState == EContextState::IDLE)
        mState = EContextState::RUNNING;

    std::queue<std::unique_ptr<IScheduleNode>> queue;
    mQueue->SwapTo(queue);

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}] - Begin Schedule, Length[{}]",
        __FUNCTION__, static_cast<void *>(this), GetServiceName(), queue.size());

    // Schedule Loop
    while (!queue.empty() && mState < EContextState::WAITING) {
        const auto node = std::move(queue.front());
        queue.pop();

        // Check Waiting To Shut Down Again
        if (mState >= EContextState::WAITING) {
            shutdown();
            return;
        }

        try {
            node->Execute();
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
        }
    }

    // Do The Same Check Likes In The Loop
    if (mState >= EContextState::WAITING) {
        shutdown();
        return;
    }

    // Schedule The New Nodes In Other Coroutine
    if (!mQueue->IsEmpty()) {
        co_spawn(GetServer()->GetIOContext(), [weak = weak_from_this()]() -> awaitable<void> {
            if (const auto self = weak.lock()) {
                self->DoSchedule();
            }
            co_return;
        }, detached);

        SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}] - Schedule Again",
            __FUNCTION__, static_cast<void *>(this), GetServiceName());
    } else {
        mState = EContextState::IDLE;
        SPDLOG_TRACE("{:<20} - Context[{:p}], Service[Name: {}] - End Schedule",
            __FUNCTION__, static_cast<void *>(this), GetServiceName());
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
    mQueue = std::make_unique<AScheduleQueue>();

    // Create Package Pool For Data Exchange
    mPool = GetServer()->CreatePackagePool(GetServer()->GetIOContext());
    mPool->Initial();

    // Initialize The Service With Package
    mService->SetUpContext(this);
    mService->Initial(pkg);

    // Context And Service Initialized
    mState = EContextState::INITIALIZED;
    SPDLOG_TRACE("{:<20} - Context[{:p}] Create Service[{}] Success",
        __FUNCTION__, static_cast<const void *>(this), mService->GetServiceName());

    return true;
}

int IContext::Shutdown(const bool bFource, const int second, const std::function<void(IContext *)> &cb) {
    // State Maybe WAITING While If Not Force To Shut Down
    if (bFource ? mState >= EContextState::SHUTTING_DOWN : mState >= EContextState::WAITING)
        return -1;

    // If Not Force To Shut Down, Turn To Waiting Current Schedule Node Execute Complete
    if (!bFource && mState == EContextState::RUNNING) {
        mState = EContextState::WAITING;

        mShutdownTimer = std::make_shared<ASystemTimer>(GetServer()->GetIOContext());
        if (cb != nullptr)
            mShutdownCallback = cb;

        // Spawn Coroutine For Waiting To Force Shut Down
        co_spawn(GetServer()->GetIOContext(), [weak = weak_from_this(), timer = mShutdownTimer, second]() -> awaitable<void> {
            timer->expires_after(std::chrono::seconds(second));
            if (const auto [ec] = co_await timer->async_wait(); ec)
                co_return;

            if (const auto self = weak.lock()) {
                if (self->GetState() == EContextState::WAITING) {
                    self->ForceShutdown();
                }
            }
        }, detached);

        return 0;
    }

    mState = EContextState::SHUTTING_DOWN;
    mShutdownTimer.reset();

    const std::string name = GetServiceName();

    if (mService && mService->GetState() != EServiceState::TERMINATED) {
        mService->Stop();
    }

    if (mQueue != nullptr && !mQueue->IsEmpty())
        mQueue->Clear();

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
    SPDLOG_TRACE("{:<20} - Context[{:p}] Service[{}] Shut Down Success",
        __FUNCTION__, static_cast<void *>(this), name);

    if (mShutdownCallback) {
        std::invoke(mShutdownCallback, this);
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

    // Handle The Package Received Before Starting
    if (!mQueue->IsEmpty()) {
        mState = EContextState::RUNNING;
        co_spawn(GetServer()->GetIOContext(), [weak = weak_from_this()]() -> awaitable<void> {
            if (const auto self = weak.lock()) {
                self->DoSchedule();
            }
            co_return;
        }, detached);
    }

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

    auto node = std::make_unique<UPackageNode>(mService);
    node->SetPackage(pkg);

    PushNode(std::move(node));
}

void IContext::PushTask(const std::function<void(IService *)> &task) {
    // As Same As ::PushPackage()
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    if (task == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}]",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName());

    auto node = std::make_unique<UTaskNode>(mService);
    node->SetTask(task);

    PushNode(std::move(node));
}

void IContext::PushEvent(const std::shared_ptr<IEventParam> &event) {
    // As Same As ::PushPackage()
    if (mState < EContextState::INITIALIZED || mState >= EContextState::WAITING)
        return;

    if (event == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}] - Event Type {}",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName(), event->GetEventType());

    auto node = std::make_unique<UEventNode>(mService);
    node->SetEventParam(event);

    PushNode(std::move(node));
}

void IContext::SendCommand(const std::string &type, const std::string &args, const std::string &comment) const {
    if (mState < EContextState::INITIALIZED)
        return;

    auto *monitor = GetServer()->GetModule<UMonitor>();
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
    if (const auto *service = GetServer()->GetModule<UServiceModule>()) {
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

    if (const auto *service = GetServer()->GetModule<UServiceModule>()) {
        if (const auto sid = service->GetServiceID(name); sid != GetServiceID())
            return sid;
        return -14;
    }

    return -13;
}

IModule *IContext::GetModuleByName(const std::string &name) const {
    return GetServer()->GetModuleByName(name);
}
