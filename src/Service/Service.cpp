#include "Service.h"
#include "ServiceModule.h"
#include "../Gateway/Gateway.h"
#include "../Event/EventModule.h"
#include "../Config/Config.h"
#include "../Timer/TimerModule.h"
#include "../Package.h"

#include <spdlog/spdlog.h>


UContext::UContext()
    : mServiceID(INVALID_SERVICE_ID) {
}

void UContext::SetServiceID(const int32_t sid) {
    if (GetState() != EContextState::CREATED)
        return;
    mServiceID = sid;
}

int32_t UContext::GetServiceID() const {
    return mServiceID;
}

UServiceModule *UContext::GetServiceModule() const {
    return dynamic_cast<UServiceModule *>(GetOwner());
}

IService::IService()
    : mContext(nullptr),
      mState(EServiceState::CREATED) {
}

void IService::SetUpContext(IContext *context) {
    if (mState != EServiceState::CREATED)
        return;
    mContext = context;
}

int32_t IService::GetServiceID() const {
    if (mContext == nullptr)
        return INVALID_SERVICE_ID;
    return mContext->GetServiceID();
}

asio::io_context &IService::GetIOContext() const {
    // assert(context_ != nullptr);
    return mContext->GetServer()->GetIOContext();
}

bool IService::Initial(const std::shared_ptr<IPackage> &pkg) {
    if (mContext == nullptr) {
        SPDLOG_ERROR("{:<20} - Context Is Null", __FUNCTION__);
        return false;
    }
    mState = EServiceState::INITIALIZED;
    return true;
}

bool IService::Start() {
    mState = EServiceState::RUNNING;
    return true;
}

void IService::Stop() {
    if (mState == EServiceState::TERMINATED)
        return;

    mState = EServiceState::TERMINATED;
}

std::shared_ptr<IPackage> IService::BuildPackage() const {
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

void IService::PostPackage(const std::shared_ptr<IPackage> &pkg) const {
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

void IService::PostPackage(const std::string &name, const std::shared_ptr<IPackage> &pkg) const {
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

void IService::PostTask(const int32_t target, const std::function<void(IService *)> &task) const {
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

void IService::PostTask(const std::string &name, const std::function<void(IService *)> &task) const {
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

void IService::SendToPlayer(const int64_t pid, const std::shared_ptr<IPackage> &pkg) const {
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

void IService::PostToPlayer(int64_t pid, const std::function<void(IService *)> &task) const {
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

void IService::SendToClient(const int64_t pid, const std::shared_ptr<IPackage> &pkg) const {
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

void IService::OnPackage(const std::shared_ptr<IPackage> &pkg) {
}

void IService::OnEvent(const std::shared_ptr<IEventParam> &event) {
}

void IService::CloseSelf() {
    if (mState != EServiceState::RUNNING)
        return;

    SendCommand("SHUTDOWN", std::to_string(GetServiceID()), "It Is Time ToClose");
}

void IService::SendCommand(const std::string &type, const std::string &args, const std::string &comment) const {
    if (mState != EServiceState::RUNNING)
        return;

    mContext->SendCommand(type, args, comment);
}

EServiceState IService::GetState() const {
    return mState;
}

UServer *IService::GetServer() const {
    if (mContext == nullptr)
        return nullptr;
    return mContext->GetServer();
}

std::map<std::string, int32_t> IService::GetServiceList() const {
    return mContext->GetServiceList();
}

int32_t IService::GetOtherServiceID(const std::string &name) const {
    // if (name.empty() || name == GetServiceName())
    //     return -11;

    return mContext->GetOtherServiceID(name);
}

void IService::ListenEvent(const int event) const {
    if (auto *eventModule = GetModule<UEventModule>()) {
        eventModule->ListenEvent(event, GetServiceID());
    }
}

void IService::RemoveListener(const int event) const {
    if (auto *eventModule = GetModule<UEventModule>()) {
        eventModule->RemoveListener(event, GetServiceID());
    }
}

void IService::DispatchEvent(const std::shared_ptr<IEventParam> &event) const {
    if (const auto *eventModule = GetModule<UEventModule>()) {
        eventModule->Dispatch(event);
    }
}

int64_t IService::SetTimer(const std::function<void(IService *)> &task, const int delay, const int rate) const {
    if (auto *timer = GetModule<UTimerModule>()) {
        return timer->SetTimer(GetServiceID(), -1, task, delay, rate);
    }
    return -1;
}

void IService::CancelTimer(const int64_t timerID) {
    auto *timerModule = GetModule<UTimerModule>();
    if (timerModule == nullptr)
        return;

    if (timerID > 0) {
        timerModule->CancelTimer(timerID);
    } else {
        timerModule->CancelServiceTimer(GetServiceID());
    }
}

IModule *IService::GetModuleByName(const std::string &name) const {
    if (mContext == nullptr)
        return nullptr;
    return mContext->GetModuleByName(name);
}
