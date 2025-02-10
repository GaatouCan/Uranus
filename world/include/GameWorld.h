#pragma once

#include "SubSystem.h"
#include "utils.h"

#include <queue>
#include <typeindex>
#include <yaml-cpp/yaml.h>

class IServerLogic;

using AConnectionPointer = std::shared_ptr<class UConnection>;
typedef IServerLogic*(*AServerCreator)(UGameWorld*);
typedef void(*AServerDestroyer)(IServerLogic*);

struct FStringViewHash {
    using is_transparent = void;

    std::size_t operator()(const std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }
};

struct FStringViewEqual {
    using is_transparent = void;

    bool operator()(const std::string_view lhs, const std::string_view rhs) const {
        return lhs == rhs;
    }
};

class BASE_API UGameWorld final {

    asio::io_context ctx_;
    ATcpAcceptor acceptor;

    AModuleHandle module_;
    IServerLogic *server_;
    AServerDestroyer serverDestroyer_;

    class UConfigManager *configManager_;
    class ULoginAuthenticator *loginAuthenticator_;
    class USceneManager *sceneManager_;
    class UGlobalQueue *globalQueue_;
    class UProtocolRoute *protocolRoute_;

    std::unordered_map<std::string, AConnectionPointer, FStringViewHash, FStringViewEqual> connectionMap_;

    ASystemTimer fullTimer_;

    struct FSystemPriority {
        int priority;
        std::type_index typeIndex;

        bool operator<(const FSystemPriority &other) const {
            return priority < other.priority;
        }

        bool operator>(const FSystemPriority &other) const {
            return priority > other.priority;
        }
    };

    std::priority_queue<FSystemPriority, std::vector<FSystemPriority>, std::greater<> > initPriority_;
    std::priority_queue<FSystemPriority, std::vector<FSystemPriority>, std::less<> > destPriority_;

    std::unordered_map<std::type_index, ISubSystem *> systemMap_;
    std::unordered_map<std::string, ISubSystem *, FStringViewHash, FStringViewEqual> nameToSystem_;

    // std::function<void(const AConnectionPointer &)> mConnectionFilter;

    AThreadID worldThreadId_;

    bool inited_;
    std::atomic_bool running_;

public:
    UGameWorld();
    ~UGameWorld();

    DISABLE_COPY_MOVE(UGameWorld)

    UGameWorld &Init(const std::string &dllPath);
    UGameWorld &Run();
    UGameWorld &Shutdown();

    void RemoveConnection(const std::string &key);
    void RemoveConnection(std::string_view key);

    asio::io_context &GetIOContext();

    AThreadID GetThreadID() const;
    [[nodiscard]] bool IsMainThread() const;

    template<SYSTEM_TYPE T>
    T *GetSystem() const noexcept {
        if (const auto iter = systemMap_.find(typeid(T)); iter != systemMap_.end())
            return dynamic_cast<T *>(iter->second);
        return nullptr;
    }

    [[nodiscard]] UConfigManager *GetConfigManager() const;
    [[nodiscard]] USceneManager *GetSceneManager() const;
    [[nodiscard]] ULoginAuthenticator *GetLoginAuthenticator() const;
    [[nodiscard]] UProtocolRoute *GetProtocolRoute() const;
    [[nodiscard]] UGlobalQueue *GetGlobalQueue() const;

    ISubSystem *GetSystemByName(std::string_view sys) const;

    const YAML::Node &GetServerConfig();
    uint32_t GetServerID();

private:
    bool LoadServerDLL(const std::string &path);
    awaitable<void> WaitForConnect();

    template<SYSTEM_TYPE T>
    T *CreateSystem(const int priority = 0) {
        if (inited_)
            return nullptr;

        const auto res = new T(this);

        systemMap_.insert_or_assign(typeid(T), res);
        nameToSystem_.insert_or_assign(res->GetSystemName(), res);

        initPriority_.push({ priority, typeid(T) });
        destPriority_.push({ priority, typeid(T) });

        return res;
    }
};
