#pragma once

#include "Module.h"
#include "ServerHandler.h"

#include <typeindex>
#include <absl/container/flat_hash_map.h>
#include <memory>


class IRecycler;
class ULoginAuth;
class UConnection;


class BASE_API UServer final {

    // Module Define Here
    absl::flat_hash_map<std::type_index, std::unique_ptr<IModule>> mModuleMap;
    absl::flat_hash_map<std::string, std::type_index> mNameToType;
    std::vector<std::type_index> mModuleOrder;

    // Service Define
    asio::io_context mContext;
    asio::executor_work_guard<asio::io_context::executor_type> mGuard;

    std::vector<std::thread> mWorkerList;

    std::unique_ptr<IServerHandler> mHandler;

    std::atomic_bool bInitialized;
    std::atomic_bool bRunning;
    std::atomic_bool bShutdown;

public:
    UServer();
    ~UServer();

    DISABLE_COPY_MOVE(UServer)

    template<CModuleType Module, typename... Args>
    Module *CreateModule(Args &&... args);

    template<CModuleType Module>
    Module *GetModule() const;

    IModule *GetModuleByName(const std::string &name) const;

    [[nodiscard]] asio::io_context &GetIOContext();

    template<class Type>
    requires std::derived_from<Type, IServerHandler>
    void SetServerHandler();

    [[nodiscard]] IServerHandler *GetServerHandler() const;

    std::shared_ptr<IRecycler> CreatePackagePool(asio::io_context &ctx) const;

    void InitLoginAuth(ULoginAuth *auth) const;
    void InitConnection(const std::shared_ptr<UConnection> &conn) const;

    void Initial();
    void Run();
    void Shutdown();

    [[nodiscard]] bool IsInitialized() const;
    [[nodiscard]] bool IsRunning() const;
    [[nodiscard]] bool IsShutdown() const;
};

template<CModuleType Module, typename ... Args>
inline Module *UServer::CreateModule(Args &&...args) {
    if (bInitialized || bRunning || bShutdown) {
        return nullptr;
    }

    if (mModuleMap.contains(typeid(Module))) {
        return dynamic_cast<Module *>(mModuleMap[typeid(Module)].get());
    }

    auto module = new Module(this, std::forward<Args>(args)...);
    auto ptr = std::unique_ptr<IModule>(module);

    mNameToType.emplace(ptr->GetModuleName(), typeid(Module));
    mModuleOrder.emplace_back(typeid(Module));

    mModuleMap.emplace(typeid(Module), std::move(ptr));

    return module;
}

template<CModuleType Module>
inline Module *UServer::GetModule() const {
    const auto iter = mModuleMap.find(typeid(Module));
    return iter != mModuleMap.end() ? dynamic_cast<Module *>(iter->second.get()) : nullptr;
}

template<class Type>
requires std::derived_from<Type, IServerHandler>
inline void UServer::SetServerHandler() {
    mHandler = std::make_unique<Type>();
}
