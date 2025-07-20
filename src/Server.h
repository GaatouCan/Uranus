#pragma once

#include "Module.h"
#include "ServerHandler.h"

#include <typeindex>
#include <absl/container/flat_hash_map.h>
#include <memory>


class IRecyclerBase;
class ULoginAuth;
class UConnection;


class BASE_API UServer final {

#pragma region Module Define
    absl::flat_hash_map<std::type_index, std::unique_ptr<IModuleBase>> ModuleMap;
    absl::flat_hash_map<std::string, std::type_index> NameToType;
    std::vector<std::type_index> ModuleOrdered;
#pragma endregion

#pragma region Worker
    asio::io_context IOContext;
    asio::executor_work_guard<asio::io_context::executor_type> ExecutorWorkGuard;
    std::vector<std::thread> WorkerList;
#pragma endregion

    std::unique_ptr<IServerHandler> Handler;

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

    IModuleBase *GetModule(const std::string &name) const;

    [[nodiscard]] asio::io_context &GetIOContext();

    template<class Type>
    requires std::derived_from<Type, IServerHandler>
    void SetServerHandler();

    [[nodiscard]] IServerHandler *GetServerHandler() const;

    std::shared_ptr<IRecyclerBase> CreatePackagePool(asio::io_context &ctx) const;

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

    if (ModuleMap.contains(typeid(Module))) {
        return dynamic_cast<Module *>(ModuleMap[typeid(Module)].get());
    }

    auto module = new Module(std::forward<Args>(args)...);
    auto ptr = std::unique_ptr<IModuleBase>(module);

    ptr->SetUpServer(this);

    NameToType.emplace(ptr->GetModuleName(), typeid(Module));
    ModuleOrdered.emplace_back(typeid(Module));

    ModuleMap.emplace(typeid(Module), std::move(ptr));

    return module;
}

template<CModuleType Module>
inline Module *UServer::GetModule() const {
    const auto iter = ModuleMap.find(typeid(Module));
    return iter != ModuleMap.end() ? dynamic_cast<Module *>(iter->second.get()) : nullptr;
}

template<class Type>
requires std::derived_from<Type, IServerHandler>
inline void UServer::SetServerHandler() {
    Handler = std::make_unique<Type>();
}
