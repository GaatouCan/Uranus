#include "Service.h"
#include "ServiceModule.h"
#include "Gateway/Gateway.h"
#include "Event/EventModule.h"
#include "Config/Config.h"
#include "Timer/TimerModule.h"
#include "Package.h"

#include <spdlog/sinks/daily_file_sink.h>


UContext::UContext()
    : serviceId_(INVALID_SERVICE_ID) {
}

void UContext::SetServiceID(const int32_t sid) {
    if (GetState() != EContextState::CREATED)
        return;
    serviceId_ = sid;
}

int32_t UContext::GetServiceID() const {
    return serviceId_;
}

UServiceModule *UContext::GetServiceModule() const {
    return dynamic_cast<UServiceModule *>(GetOwner());
}

IServiceBase::IServiceBase()
    : mContext(nullptr),
      mState(EServiceState::CREATED) {
}

void IServiceBase::SetUpContext(IContextBase *context) {
    if (mState != EServiceState::CREATED)
        return;
    mContext = context;
}

int32_t IServiceBase::GetServiceID() const {
    if (mContext == nullptr)
        return INVALID_SERVICE_ID;
    return mContext->GetServiceID();
}

std::string IServiceBase::GetServiceName() const {
    return {};
}

// std::shared_ptr<spdlog::logger> IServiceBase::CreateLogger(const std::string &name, const std::string &path) {
//     if (state_ == EServiceState::TERMINATED)
//         return nullptr;
//
//     if (loggerSet_.contains(name)) {
//         return GetLogger(name);
//     }
//
//     const auto serviceName = GetServiceName();
//     if (serviceName.empty() || serviceName == "UNKNOWN") {
//         SPDLOG_ERROR("{:<20} - Unknown Service Name: Service ID {}", __FUNCTION__, GetServiceID());
//         return nullptr;
//     }
//
//     const auto *config = GetModule<UConfig>();
//     if (!config)
//         return nullptr;
//
//     const auto &cfg = config->GetServerConfig();
//     const auto rootDir = cfg["server"]["logger_dir"].as<std::string>();
//
//     const auto loggerPath = rootDir + "/" + serviceName + "/" + path;
//     const auto loggerName = fmt::format("{} - {}", serviceName, name);
//
//     auto logger = spdlog::daily_logger_mt(loggerName, loggerPath, 2, 0);
//     loggerSet_.insert(name);
//
//     return logger;
// }
//
// void IServiceBase::CreateLogger(const std::map<std::string, std::string> &loggers) {
//     if (state_ == EServiceState::TERMINATED)
//         return;
//
//     const auto serviceName = GetServiceName();
//     if (serviceName.empty() || serviceName == "UNKNOWN") {
//         SPDLOG_ERROR("{:<20} - Unknown Service Name: Service ID {}", __FUNCTION__, GetServiceID());
//         return;
//     }
//
//     const auto *config = GetModule<UConfig>();
//     if (!config)
//         return;
//
//     const auto &cfg = config->GetServerConfig();
//     const auto rootDir = cfg["server"]["logger_dir"].as<std::string>();
//
//     for (const auto &[name, path] : loggers) {
//         if (loggerSet_.contains(name))
//             continue;
//
//         auto loggerPath = rootDir;
//
//         loggerPath += "/";
//         loggerPath += serviceName;
//         loggerPath += "/";
//         loggerPath += path;
//
//         const auto loggerName = fmt::format("{} - {}", serviceName, name);
//
//         spdlog::daily_logger_mt(loggerName, loggerPath, 2, 0);
//         loggerSet_.insert(name);
//     }
// }
//
// std::shared_ptr<spdlog::logger> IServiceBase::GetLogger(const std::string &name) const {
//     if (state_ == EServiceState::TERMINATED)
//         return nullptr;
//
//     if (!loggerSet_.contains(name))
//         return nullptr;
//
//     const auto serviceName = GetServiceName();
//     if (serviceName.empty() || serviceName == "UNKNOWN")
//         return nullptr;
//
//     const auto loggerName = fmt::format("{} - {}", serviceName, name);
//     auto result = spdlog::get(loggerName);
//
//     return result;
// }

io_context &IServiceBase::GetIOContext() const {
    // assert(context_ != nullptr);
    return mContext->GetServer()->GetIOContext();
}

bool IServiceBase::Initial(const std::shared_ptr<IPackageInterface> &pkg) {
    if (mState != EServiceState::CREATED)
        return false;

    if (mContext == nullptr) {
        SPDLOG_ERROR("{:<20} - Context Is Null", __FUNCTION__);
        return false;
    }
    mState = EServiceState::INITIALIZED;
    return true;
}

bool IServiceBase::Start() {
    mState = EServiceState::RUNNING;
    return true;
}

void IServiceBase::Stop() {
    if (mState == EServiceState::TERMINATED)
        return;

    mState = EServiceState::TERMINATED;

    // Release All The Loggers Created By This Service
    // if (const auto serviceName = GetServiceName(); !serviceName.empty() && serviceName != "UNKNOWN") {
    //     for (const auto &val : loggerSet_) {
    //         const auto loggerName = fmt::format("{} - {}", serviceName, val);
    //         spdlog::drop(loggerName);
    //     }
    // }
}

std::shared_ptr<IPackageInterface> IServiceBase::BuildPackage() const {
    if (mState != EServiceState::RUNNING)
        return nullptr;

    if (mContext == nullptr)
        return nullptr;

    if (auto pkg = mContext->BuildPackage()) {
        pkg->SetSource(GetServiceID());
        return pkg;
    }

    return nullptr;
}

void IServiceBase::PostPackage(const std::shared_ptr<IPackageInterface> &pkg) const {
    if (mState != EServiceState::RUNNING)
        return;

    if (pkg == nullptr)
        return;

    // Do Not Post To Self
    const auto target = pkg->GetTarget();
    if (target <= 0 || target == GetServiceID())
        return;

    const auto *module = GetModule<UServiceModule>();
    if (module == nullptr)
        return;

    if (const auto context = module->FindService(target)) {
        SPDLOG_TRACE("{:<20} - From Service[ID: {}, Name: {}] To Service[ID: {}, Name: {}]",
        __FUNCTION__, GetServiceID(), GetServiceName(), target, context->GetServiceName());

        pkg->SetSource(GetServiceID());
        context->PushPackage(pkg);
    }
}

void IServiceBase::PostPackage(const std::string &name, const std::shared_ptr<IPackageInterface> &pkg) const {
    if (mState != EServiceState::RUNNING)
        return;

    // Do Not Post To Self
    if (pkg == nullptr || name == GetServiceName())
        return;

    const auto *module = GetModule<UServiceModule>();
    if (module == nullptr)
        return;

    if (const auto target = module->FindService(name)) {
        SPDLOG_TRACE("{:<20} - From Service[ID: {}, Name: {}] To Service[ID: {}, Name: {}]",
            __FUNCTION__, GetServiceID(), GetServiceName(), target->GetServiceID(), target->GetServiceName());

        pkg->SetSource(GetServiceID());
        pkg->SetTarget(target->GetServiceID());

        target->PushPackage(pkg);
    }
}

void IServiceBase::PostTask(const int32_t target, const std::function<void(IServiceBase *)> &task) const {
    if (mState != EServiceState::RUNNING)
        return;

    // Do Not Post To Self
    if (target < 0 || target == GetServiceID())
        return;

    const auto *module = GetModule<UServiceModule>();
    if (module == nullptr)
        return;

    if (const auto context = module->FindService(target)) {
        SPDLOG_TRACE("{:<20} - From Service[ID: {}, Name: {}] To Service[ID: {}, Name: {}]",
            __FUNCTION__, GetServiceID(), GetServiceName(), target, context->GetServiceName());

        context->PushTask(task);
    }
}

void IServiceBase::PostTask(const std::string &name, const std::function<void(IServiceBase *)> &task) const {
    if (mState != EServiceState::RUNNING)
        return;

    // Do Not Post To Self
    if (name == GetServiceName())
        return;

    const auto *module = GetModule<UServiceModule>();
    if (module == nullptr)
        return;

    if (const auto target = module->FindService(name)) {
        SPDLOG_TRACE("{:<20} - From Service[ID: {}, Name: {}] To Service[ID: {}, Name: {}]",
            __FUNCTION__, GetServiceID(), GetServiceName(), target->GetServiceID(), target->GetServiceName());

        target->PushTask(task);
    }
}

void IServiceBase::SendToPlayer(const int64_t pid, const std::shared_ptr<IPackageInterface> &pkg) const {
    if (mState != EServiceState::RUNNING)
        return;

    if (pkg == nullptr)
        return;

    if (const auto *gateway = GetModule<UGateway>()) {
        SPDLOG_TRACE("{:<20} - From Service[ID: {}, Name: {}] To Player[{}]",
        __FUNCTION__, GetServiceID(), GetServiceName(), pid);

        pkg->SetSource(GetServiceID());
        pkg->SetTarget(PLAYER_AGENT_ID);

        gateway->SendToPlayer(pid, pkg);
    }
}

void IServiceBase::PostToPlayer(int64_t pid, const std::function<void(IServiceBase *)> &task) const {
    if (mState != EServiceState::RUNNING)
        return;

    if (task == nullptr)
        return;

    if (const auto *gateway = GetModule<UGateway>()) {
        SPDLOG_TRACE("{:<20} - From Service[ID: {}, Name: {}] To Player[{}]",
         __FUNCTION__, GetServiceID(), GetServiceName(), pid);

        gateway->PostToPlayer(pid, task);
    }
}

void IServiceBase::SendToClient(const int64_t pid, const std::shared_ptr<IPackageInterface> &pkg) const {
    if (mState != EServiceState::RUNNING)
        return;

    if (pkg == nullptr)
        return;

    if (const auto *gateway = GetModule<UGateway>()) {
        SPDLOG_TRACE("{:<20} - From Service[ID: {}, Name: {}] To Player[{}]",
        __FUNCTION__, GetServiceID(), GetServiceName(), pid);

        pkg->SetSource(GetServiceID());
        pkg->SetTarget(CLIENT_TARGET_ID);

        gateway->SendToClient(pid, pkg);
    }
}

void IServiceBase::OnPackage(const std::shared_ptr<IPackageInterface> &pkg) {
}

void IServiceBase::OnEvent(const std::shared_ptr<IEventInterface> &event) {
}

void IServiceBase::CloseSelf() {
    if (mState != EServiceState::RUNNING)
        return;

    SendCommand("SHUTDOWN", std::to_string(GetServiceID()), "It Is Time ToClose");
}

void IServiceBase::SendCommand(const std::string &type, const std::string &args, const std::string &comment) const {
    if (mState != EServiceState::RUNNING)
        return;

    mContext->SendCommand(type, args, comment);
}

EServiceState IServiceBase::GetState() const {
    return mState;
}

UServer *IServiceBase::GetServer() const {
    if (mContext == nullptr)
        return nullptr;
    return mContext->GetServer();
}

std::map<std::string, int32_t> IServiceBase::GetServiceList() const {
    return mContext->GetServiceList();
}

int32_t IServiceBase::GetOtherServiceID(const std::string &name) const {
    // if (name.empty() || name == GetServiceName())
    //     return -11;

    return mContext->GetOtherServiceID(name);
}

void IServiceBase::ListenEvent(const int event) const {
    if (auto *eventModule = GetModule<UEventModule>()) {
        eventModule->ListenEvent(event, GetServiceID());
    }
}

void IServiceBase::RemoveListener(const int event) const {
    if (auto *eventModule = GetModule<UEventModule>()) {
        eventModule->RemoveListener(event, GetServiceID());
    }
}

void IServiceBase::DispatchEvent(const std::shared_ptr<IEventInterface> &event) const {
    if (const auto *eventModule = GetModule<UEventModule>()) {
        eventModule->Dispatch(event);
    }
}

FTimerHandle IServiceBase::SetSteadyTimer(const std::function<void(IServiceBase *)> &task, const int delay, const int rate) const {
    if (auto *timer = GetModule<UTimerModule>()) {
        return timer->SetSteadyTimer(GetServiceID(), -1, task, delay, rate);
    }
    return { -1, true };
}

FTimerHandle IServiceBase::SetSystemTimer(const std::function<void(IServiceBase *)> &task, int delay, int rate) const {
    if (auto *timer = GetModule<UTimerModule>()) {
        return timer->SetSystemTimer(GetServiceID(), -1, task, delay, rate);
    }
    return { -1, false };
}

void IServiceBase::CancelTimer(const FTimerHandle &handle) {
    auto *timerModule = GetModule<UTimerModule>();
    if (timerModule == nullptr)
        return;

    if (handle.id > 0) {
        timerModule->CancelTimer(handle);
    } else {
        timerModule->CancelServiceTimer(GetServiceID());
    }
}

IModuleBase *IServiceBase::GetModule(const std::string &name) const {
    if (mContext == nullptr)
        return nullptr;
    return mContext->GetModule(name);
}

std::optional<nlohmann::json> IServiceBase::FindConfig(const std::string &path) const {
    if (const auto *config = GetModule<UConfig>()) {
        return config->Find(path);
    }
    return std::nullopt;
}
