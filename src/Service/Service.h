#pragma once

#include "../Context.h"
#include "../Event/EventParam.h"

#include <functional>
#include <memory>


class UServiceModule;
class IPackage;
class IProtoRoute;


class BASE_API UContext final : public IContext {

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


class BASE_API IService {

public:
    IService();
    virtual ~IService() = default;

    DISABLE_COPY_MOVE(IService)

    void SetUpContext(IContext *context);

    [[nodiscard]] int32_t GetServiceID() const;
    [[nodiscard]] virtual std::string GetServiceName() const = 0;

    [[nodiscard]] asio::io_context &GetIOContext() const;

    virtual bool Initial(const std::shared_ptr<IPackage> &pkg);
    virtual bool Start();
    virtual void Stop();

    /// Get An Unused Package From Package Pool
    std::shared_ptr<IPackage> BuildPackage() const;

    /// Send To Other Service Use Target In Package
    void PostPackage(const std::shared_ptr<IPackage> &pkg) const;

    /// Send To Other Service Use Service Name
    void PostPackage(const std::string &name, const std::shared_ptr<IPackage> &pkg) const;

    void PostTask(int32_t target, const std::function<void(IService *)> &task) const;
    void PostTask(const std::string &name, const std::function<void(IService *)> &task) const;

    template<class Type, class Callback, class... Args>
    requires std::derived_from<Type, IService>
    void PostTaskT(int32_t target, Callback &&func, Args &&... args);

    template<class Type, class Callback, class... Args>
    requires std::derived_from<Type, IService>
    void PostTaskT(const std::string &name, Callback &&func, Args &&... args);

    virtual void SendToPlayer(int64_t pid, const std::shared_ptr<IPackage> &pkg) const;
    virtual void PostToPlayer(int64_t pid, const std::function<void(IService *)> &task) const;

    template<class Type, class Callback, class... Args>
    requires std::derived_from<Type, IService>
    void PostToPlayerT(int64_t pid, Callback &&func, Args &&... args);

    virtual void SendToClient(int64_t pid, const std::shared_ptr<IPackage> &pkg) const;

    virtual void OnPackage(const std::shared_ptr<IPackage> &pkg);
    virtual void OnEvent(const std::shared_ptr<IEventParam> &event);

    virtual void CloseSelf();

    void SendCommand(const std::string &type, const std::string &args, const std::string &comment = "") const;

    [[nodiscard]] EServiceState GetState() const;

    [[nodiscard]] std::map<std::string, int32_t> GetServiceList() const;
    [[nodiscard]] int32_t GetOtherServiceID(const std::string &name) const;

    // std::optional<nlohmann::json> FindConfig(const std::string &path, uint64_t id) const;

    virtual void ListenEvent(int event) const;
    virtual void RemoveListener(int event) const;

    template<CEventType Event>
    std::shared_ptr<Event> CreateEventParam();

    void DispatchEvent(const std::shared_ptr<IEventParam> &event) const;

    template<CEventType Event, class... Args>
    void DispatchEventT(Args && ... args);

    virtual int64_t SetTimer(const std::function<void(IService *)> &task, int delay, int rate) const;
    virtual void CancelTimer(int64_t timerID);

    [[nodiscard]] UServer *GetServer() const;

    template<CModuleType Module>
    Module *GetModule() const;

    IModule *GetModuleByName(const std::string &name) const;

protected:
    IContext *mContext;
    std::atomic<EServiceState> mState;
};


template<class Type, class Callback, class ... Args>
requires std::derived_from<Type, IService>
inline void IService::PostTaskT(int32_t target, Callback &&func, Args &&...args) {
    auto task = [func = std::forward<Callback>(func), ...args = std::forward<Args>(args)](IService *ser) {
        auto *ptr = dynamic_cast<Type *>(ser);
        if (ptr == nullptr)
            return;

        std::invoke(func, ptr, args...);
    };
    this->PostTask(target, task);
}

template<class Type, class Callback, class ... Args> requires std::derived_from<Type, IService>
inline void IService::PostTaskT(const std::string &name, Callback &&func, Args &&...args) {
    auto task = [func = std::forward<Callback>(func), ...args = std::forward<Args>(args)](IService *ser) {
        auto *ptr = dynamic_cast<Type *>(ser);
        if (ptr == nullptr)
            return;

        std::invoke(func, ptr, args...);
    };
    this->PostTask(name, task);
}

template<class Type, class Callback, class ... Args> requires std::derived_from<Type, IService>
inline void IService::PostToPlayerT(int64_t pid, Callback &&func, Args &&...args) {
    auto task = [func = std::forward<Callback>(func), ...args = std::forward<Args>(args)](IService *ser) {
        auto *ptr = dynamic_cast<Type *>(ser);
        if (ptr == nullptr)
            return;

        std::invoke(func, ptr, args...);
    };
    this->PostToPlayer(pid, task);
}

template<CEventType Event>
inline std::shared_ptr<Event> IService::CreateEventParam() {
    return std::make_shared<Event>();
}

template<CEventType Event, class ... Args>
inline void IService::DispatchEventT(Args &&...args) {
    auto event = std::make_shared<Event>(std::forward<Args>(args)...);
    this->DispatchEvent(event);
}

template<CModuleType Module>
Module *IService::GetModule() const {
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
