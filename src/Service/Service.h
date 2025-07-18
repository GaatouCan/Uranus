#pragma once

#include "Context.h"
#include "Event/EventParam.h"

#include <functional>
#include <memory>
#include <absl/container/flat_hash_set.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>


class UServiceModule;
class IPackageBase;
struct FTimerHandle;


class BASE_API UContext final : public IContextBase {

    int32_t mServiceID;

public:
    UContext();
    ~UContext() override = default;

    void SetServiceID(int32_t sid);
    int32_t GetServiceID() const override;

    UServiceModule *GetServiceModule() const;
};


enum class BASE_API EServiceState {
    CREATED,
    INITIALIZED,
    RUNNING,
    TERMINATED,
};


class BASE_API IServiceBase {

public:
    IServiceBase();
    virtual ~IServiceBase() = default;

    DISABLE_COPY_MOVE(IServiceBase)

    void SetUpContext(IContextBase *context);

    [[nodiscard]] int32_t GetServiceID() const;
    [[nodiscard]] virtual std::string GetServiceName() const;

    [[nodiscard]] io_context &GetIOContext() const;

    virtual bool Initial(const std::shared_ptr<IPackageBase> &pkg);
    virtual bool Start();
    virtual void Stop();

    [[nodiscard]] EServiceState GetState() const;

    /// Get An Unused Package From Package Pool
    std::shared_ptr<IPackageBase> BuildPackage() const;

#pragma region Package
    /// Send To Other Service Use Target In Package
    void PostPackage(const std::shared_ptr<IPackageBase> &pkg) const;

    /// Send To Other Service Use Service Name
    void PostPackage(const std::string &name, const std::shared_ptr<IPackageBase> &pkg) const;
#pragma endregion

#pragma region Task
    void PostTask(int32_t target, const std::function<void(IServiceBase *)> &task) const;
    void PostTask(const std::string &name, const std::function<void(IServiceBase *)> &task) const;

    template<class Type, class Callback, class... Args>
    requires std::derived_from<Type, IServiceBase>
    void PostTaskT(int32_t target, Callback &&func, Args &&... args);

    template<class Type, class Callback, class... Args>
    requires std::derived_from<Type, IServiceBase>
    void PostTaskT(const std::string &name, Callback &&func, Args &&... args);
#pragma endregion

#pragma region ToPlayer
    virtual void SendToPlayer(int64_t pid, const std::shared_ptr<IPackageBase> &pkg) const;
    virtual void PostToPlayer(int64_t pid, const std::function<void(IServiceBase *)> &task) const;

    template<class Type, class Callback, class... Args>
    requires std::derived_from<Type, IServiceBase>
    void PostToPlayerT(int64_t pid, Callback &&func, Args &&... args);
#pragma endregion

    virtual void SendToClient(int64_t pid, const std::shared_ptr<IPackageBase> &pkg) const;

    virtual void OnPackage(const std::shared_ptr<IPackageBase> &pkg);
    virtual void OnEvent(const std::shared_ptr<IEventParam> &event);

    virtual void CloseSelf();

    void SendCommand(const std::string &type, const std::string &args, const std::string &comment = "") const;

    [[nodiscard]] std::map<std::string, int32_t> GetServiceList() const;
    [[nodiscard]] int32_t GetOtherServiceID(const std::string &name) const;

#pragma region Event
    virtual void ListenEvent(int event) const;
    virtual void RemoveListener(int event) const;

    template<CEventType Event>
    std::shared_ptr<Event> CreateEventParam();

    void DispatchEvent(const std::shared_ptr<IEventParam> &event) const;

    template<CEventType Event, class... Args>
    void DispatchEventT(Args && ... args);
#pragma endregion

#pragma region Timer
    virtual FTimerHandle SetSteadyTimer(const std::function<void(IServiceBase *)> &task, int delay, int rate) const;
    virtual FTimerHandle SetSystemTimer(const std::function<void(IServiceBase *)> &task, int delay, int rate) const;
    virtual void CancelTimer(const FTimerHandle &handle);
#pragma endregion

    [[nodiscard]] UServer *GetServer() const;

    template<CModuleType Module>
    Module *GetModule() const;

    IModuleBase *GetModule(const std::string &name) const;

    [[nodiscard]] std::optional<nlohmann::json> FindConfig(const std::string &path) const;

#pragma region Logger
    /** Create Logger For This Service */
    std::shared_ptr<spdlog::logger> CreateLogger(const std::string &name, const std::string &path);

    void CreateLogger(const std::map<std::string, std::string> &loggers);

    /** Get Logger Pointer Which Registered To This Service */
    std::shared_ptr<spdlog::logger> GetLogger(const std::string &name) const;
#pragma endregion

protected:
    IContextBase *mContext;

    /** Record That Which Logger Is For This Service */
    absl::flat_hash_set<std::string> mLoggerSet;

    std::atomic<EServiceState> mState;
};


template<class Type, class Callback, class ... Args>
requires std::derived_from<Type, IServiceBase>
inline void IServiceBase::PostTaskT(int32_t target, Callback &&func, Args &&...args) {
    auto task = [func = std::forward<Callback>(func), ...args = std::forward<Args>(args)](IServiceBase *ser) {
        auto *ptr = dynamic_cast<Type *>(ser);
        if (ptr == nullptr)
            return;

        std::invoke(func, ptr, args...);
    };
    this->PostTask(target, task);
}

template<class Type, class Callback, class ... Args> requires std::derived_from<Type, IServiceBase>
inline void IServiceBase::PostTaskT(const std::string &name, Callback &&func, Args &&...args) {
    auto task = [func = std::forward<Callback>(func), ...args = std::forward<Args>(args)](IServiceBase *ser) {
        auto *ptr = dynamic_cast<Type *>(ser);
        if (ptr == nullptr)
            return;

        std::invoke(func, ptr, args...);
    };
    this->PostTask(name, task);
}

template<class Type, class Callback, class ... Args> requires std::derived_from<Type, IServiceBase>
inline void IServiceBase::PostToPlayerT(int64_t pid, Callback &&func, Args &&...args) {
    auto task = [func = std::forward<Callback>(func), ...args = std::forward<Args>(args)](IServiceBase *ser) {
        auto *ptr = dynamic_cast<Type *>(ser);
        if (ptr == nullptr)
            return;

        std::invoke(func, ptr, args...);
    };
    this->PostToPlayer(pid, task);
}

template<CEventType Event>
inline std::shared_ptr<Event> IServiceBase::CreateEventParam() {
    return std::make_shared<Event>();
}

template<CEventType Event, class ... Args>
inline void IServiceBase::DispatchEventT(Args &&...args) {
    auto event = std::make_shared<Event>(std::forward<Args>(args)...);
    this->DispatchEvent(event);
}

template<CModuleType Module>
Module *IServiceBase::GetModule() const {
    if (mContext == nullptr)
        return nullptr;
    return mContext->GetModule<Module>();
}


#define DECLARE_SERVICE(service, base) \
private: \
    using Super = base;\
public: \
    DISABLE_COPY_MOVE(service) \
private:
