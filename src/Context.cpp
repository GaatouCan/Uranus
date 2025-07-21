#include "Context.h"
#include "Recycler.h"
#include "PackageInterface.h"
#include "Service/LibraryHandle.h"
#include "Service/ServiceModule.h"
#include "Service/Service.h"
#include "Event/EventInterface.h"
#include "Monitor/Monitor.h"

#include <spdlog/spdlog.h>


typedef IServiceBase *(*AServiceCreator)();
typedef void (*AServiceDestroyer)(IServiceBase *);


IContextBase::INodeBase::INodeBase(IServiceBase *service)
    : service_(service) {
}

IServiceBase *IContextBase::INodeBase::GetService() const {
    return service_;
}

IContextBase::UPackageNode::UPackageNode(IServiceBase *service)
    : INodeBase(service),
      package_(nullptr) {
}

void IContextBase::UPackageNode::SetPackage(const shared_ptr<IPackageInterface> &pkg) {
    package_ = pkg;
}

void IContextBase::UPackageNode::Execute() {
    if (service_ && package_) {
        service_->OnPackage(package_);
    }
}

IContextBase::UTaskNode::UTaskNode(IServiceBase *service)
    : INodeBase(service) {
}

void IContextBase::UTaskNode::SetTask(const std::function<void(IServiceBase *)> &task) {
    task_ = task;
}

void IContextBase::UTaskNode::Execute() {
    if (service_ && task_) {
        std::invoke(task_, service_);
    }
}

IContextBase::UEventNode::UEventNode(IServiceBase *service)
    : INodeBase(service) {
}

void IContextBase::UEventNode::SetEventParam(const shared_ptr<IEventInterface> &event) {
    event_ = event;
}

void IContextBase::UEventNode::Execute() {
    if (service_ && event_) {
        service_->OnEvent(event_);
    }
}

IContextBase::IContextBase()
    : module_(nullptr),
      service_(nullptr),
      handle_(nullptr),
      state_(EContextState::CREATED) {
}

IContextBase::~IContextBase() {
    if (shutdownTimer_) {
        shutdownTimer_->cancel();
        ForceShutdown();
    }

    if (channel_) {
        channel_->close();
    }
}

void IContextBase::SetUpModule(IModuleBase *module) {
    if (state_ != EContextState::CREATED)
        return;
    module_ = module;
}

void IContextBase::SetUpHandle(FLibraryHandle *handle) {
    if (state_ != EContextState::CREATED)
        return;
    handle_ = handle;
}

std::string IContextBase::GetLibraryPath() const {
    if (handle_ == nullptr)
        return {};
    return handle_->GetPath();
}

std::string IContextBase::GetServiceName() const {
    if (state_ >= EContextState::INITIALIZED) {
        return service_->GetServiceName();
    }
    return "UNKNOWN";
}

IModuleBase *IContextBase::GetOwner() const {
    return module_;
}

IServiceBase *IContextBase::GetService() const {
    return service_;
}

void IContextBase::PushNode(const shared_ptr<INodeBase> &node) {
    if (state_ < EContextState::INITIALIZED || state_ >= EContextState::WAITING)
        return;

    if (node == nullptr)
        return;

    co_spawn(GetServer()->GetIOContext(), [self = shared_from_this(), node]() -> awaitable<void> {
        co_await self->channel_->async_send(std::error_code{}, node);
    }, detached);
}

awaitable<void> IContextBase::ProcessChannel() {
    if (state_ <= EContextState::INITIALIZED || state_ >= EContextState::WAITING)
        co_return;

    while (channel_->is_open()) {
        const auto [ec, node] = co_await channel_->async_receive();
        if (ec)
            co_return;

        if (node == nullptr)
            continue;

        if (state_ >= EContextState::WAITING)
            co_return;

        state_ = EContextState::RUNNING;
        try {
            node->Execute();
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
        }

        if (state_ >= EContextState::WAITING) {
            ForceShutdown();
            co_return;
        }

        state_ = EContextState::IDLE;
    }
}


bool IContextBase::Initial(const std::shared_ptr<IPackageInterface> &pkg) {
    if (state_ != EContextState::CREATED)
        return false;

    if (module_ == nullptr || handle_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Owner Module Or Library Node Is Null", __FUNCTION__);
        return false;
    }

    // Start To Create Service
    state_ = EContextState::INITIALIZING;

    auto creator = handle_->GetCreatorT<AServiceCreator>();
    if (creator == nullptr) {
        SPDLOG_ERROR("{:<20} - Can't Load Creator, Path[{}]", __FUNCTION__, handle_->GetPath());
        state_ = EContextState::CREATED;
        return false;
    }

    service_ = std::invoke(creator);
    if (service_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Can't Create Service, Path[{}]", __FUNCTION__, handle_->GetPath());
        state_ = EContextState::CREATED;
        return false;
    }

    // Create Node Queue For Schedule
    channel_ = make_unique<AContextChannel>(GetServer()->GetIOContext(), 1024);

    // Create Package Pool For Data Exchange
    pool_ = GetServer()->CreatePackagePool(GetServer()->GetIOContext());
    pool_->Initial();

    // Initialize The Service With Package
    service_->SetUpContext(this);
    service_->Initial(pkg);

    // Context And Service Initialized
    state_ = EContextState::INITIALIZED;
    SPDLOG_TRACE("{:<20} - Context[{:p}] Service[{}] Initial Successfully",
        __FUNCTION__, static_cast<const void *>(this), service_->GetServiceName());

    return true;
}

int IContextBase::Shutdown(const bool bForce, const int second, const std::function<void(IContextBase *)> &cb) {
    // State Maybe WAITING While If Not Force To Shut Down
    if (bForce ? state_ >= EContextState::SHUTTING_DOWN : state_ >= EContextState::WAITING)
        return -1;

    // If Not Force To Shut Down, Turn To Waiting Current Schedule Node Execute Complete
    if (!bForce && state_ == EContextState::RUNNING) {
        state_ = EContextState::WAITING;

        shutdownTimer_ = make_unique<ASteadyTimer>(GetServer()->GetIOContext());
        if (cb != nullptr)
            shutdownCallback_ = cb;

        // Spawn Coroutine For Waiting To Force Shut Down
        co_spawn(GetServer()->GetIOContext(), [self = shared_from_this(), second]() -> awaitable<void> {
            self->shutdownTimer_->expires_after(std::chrono::seconds(second));
            if (const auto [ec] = co_await self->shutdownTimer_->async_wait(); ec)
                co_return;

            if (self->GetState() == EContextState::WAITING) {
                self->ForceShutdown();
            }
        }, detached);

        return 0;
    }

    state_ = EContextState::SHUTTING_DOWN;

    shutdownTimer_->cancel();
    channel_->close();

    const std::string name = GetServiceName();

    if (service_ && service_->GetState() != EServiceState::TERMINATED) {
        service_->Stop();
    }

    if (handle_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Library Node Is Null", __FUNCTION__);
        state_ = EContextState::STOPPED;
        return -2;
    }

    auto destroyer = handle_->GetDestroyerT<AServiceDestroyer>();
    if (destroyer == nullptr) {
        SPDLOG_ERROR("{:<20} - Can't Load Destroyer, Path[{}]", __FUNCTION__, handle_->GetPath());
        state_ = EContextState::STOPPED;
        return -3;
    }

    std::invoke(destroyer, service_);

    service_ = nullptr;
    handle_ = nullptr;

    state_ = EContextState::STOPPED;
    SPDLOG_TRACE("{:<20} - Context[{:p}] Service[{}] Shut Down Successfully",
        __FUNCTION__, static_cast<void *>(this), name);

    if (shutdownCallback_) {
        std::invoke(shutdownCallback_, this);
    }

    return 1;
}

int IContextBase::ForceShutdown() {
    return Shutdown(true, 0, nullptr);
}


bool IContextBase::BootService() {
    if (state_ != EContextState::INITIALIZED || service_ == nullptr)
        return false;

    state_ = EContextState::IDLE;

    if (const auto res = service_->Start(); !res) {
        SPDLOG_ERROR("{:<20} - Context[{:p}], Service[{} - {}] Failed To Boot.",
            __FUNCTION__, static_cast<const void *>(this), GetServiceID(), GetServiceName());

        state_ = EContextState::INITIALIZED;
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
    return state_;
}


UServer *IContextBase::GetServer() const {
    return module_->GetServer();
}

void IContextBase::PushPackage(const std::shared_ptr<IPackageInterface> &pkg) {
    // Could Receive Package After Initialized And Before Waiting
    if (state_ < EContextState::INITIALIZED || state_ >= EContextState::WAITING)
        return;

    if (pkg == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}] - Package From {}",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName(), pkg->GetSource());

    const auto node = make_shared<UPackageNode>(service_);
    node->SetPackage(pkg);

    PushNode(node);
}

void IContextBase::PushTask(const std::function<void(IServiceBase *)> &task) {
    // As Same As ::PushPackage()
    if (state_ < EContextState::INITIALIZED || state_ >= EContextState::WAITING)
        return;

    if (task == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}]",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName());

    const auto node = make_shared<UTaskNode>(service_);
    node->SetTask(task);

    PushNode(node);
}

void IContextBase::PushEvent(const std::shared_ptr<IEventInterface> &event) {
    // As Same As ::PushPackage()
    if (state_ < EContextState::INITIALIZED || state_ >= EContextState::WAITING)
        return;

    if (event == nullptr)
        return;

    SPDLOG_TRACE("{:<20} - Context[{:p}], Service[{}] - Event Type {}",
        __FUNCTION__, static_cast<const void *>(this), GetServiceName(), event->GetEventType());

    const auto node = make_shared<UEventNode>(service_);
    node->SetEventParam(event);

    PushNode(node);
}

void IContextBase::SendCommand(const std::string &type, const std::string &args, const std::string &comment) const {
    if (state_ < EContextState::INITIALIZED)
        return;

    auto *monitor = GetModule<UMonitor>();
    if (monitor == nullptr)
        return;

    monitor->OnCommand(GetServiceName(), type, args, comment);
}


std::shared_ptr<IPackageInterface> IContextBase::BuildPackage() const {
    if (state_ != EContextState::IDLE || state_ != EContextState::RUNNING)
        return nullptr;

    if (const auto elem = pool_->Acquire())
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
    if (state_ < EContextState::INITIALIZED)
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
