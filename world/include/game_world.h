#pragma once

#include "sub_system.h"
#include "utils.h"

#include <queue>
#include <typeindex>
#include <yaml-cpp/yaml.h>

class IServerLogic;

using ConnectionPointer = std::shared_ptr<class Connection>;

typedef IServerLogic*(*ServerCreator)(GameWorld*);
typedef void(*ServerDestroyer)(IServerLogic*);

struct StringViewHash {
    using is_transparent = void;

    std::size_t operator()(const std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }
};

struct StringViewEqual {
    using is_transparent = void;

    bool operator()(const std::string_view lhs, const std::string_view rhs) const {
        return lhs == rhs;
    }
};

class BASE_API GameWorld final {

    asio::io_context mContext;
    TcpAcceptor mAcceptor;

    ModuleHandle mModule;
    IServerLogic *mServer;
    ServerDestroyer mDestroyer;

    class ConfigManager *mConfigManager;
    class LoginAuthenticator *mLoginAuthenticator;
    class SceneManager *mSceneManager;
    class GlobalQueue *mGlobalQueue;
    class ProtocolRoute *mProtocolRoute;

    std::unordered_map<std::string, ConnectionPointer, StringViewHash, StringViewEqual> mConnectionMap;

    SystemTimer mFullTimer;

    struct SystemPriority {
        int priority;
        std::type_index typeIndex;

        bool operator<(const SystemPriority &other) const {
            return priority < other.priority;
        }

        bool operator>(const SystemPriority &other) const {
            return priority > other.priority;
        }
    };

    std::priority_queue<SystemPriority, std::vector<SystemPriority>, std::greater<> > init_priority_;
    std::priority_queue<SystemPriority, std::vector<SystemPriority>, std::less<> > dest_priority_;

    std::unordered_map<std::type_index, ISubSystem *> system_map_;
    std::unordered_map<std::string, ISubSystem *, StringViewHash, StringViewEqual> name_to_system_;

    ThreadID mThreadID;

    bool bInited;
    std::atomic_bool bRunning;

public:
    GameWorld();
    ~GameWorld();

    DISABLE_COPY_MOVE(GameWorld)

    GameWorld &Init(const std::string &path);
    GameWorld &Run();
    GameWorld &Shutdown();

    void RemoveConnection(const std::string &key);
    void RemoveConnection(std::string_view key);

    asio::io_context &GetIOContext();

    ThreadID GetThreadID() const;
    [[nodiscard]] bool IsMainThread() const;

    template<SystemType T>
    T *GetSystem() const noexcept {
        if (const auto iter = system_map_.find(typeid(T)); iter != system_map_.end())
            return dynamic_cast<T *>(iter->second);
        return nullptr;
    }

    [[nodiscard]] ConfigManager *GetConfigManager() const;
    [[nodiscard]] SceneManager *GetSceneManager() const;
    [[nodiscard]] LoginAuthenticator *GetLoginAuthenticator() const;
    [[nodiscard]] ProtocolRoute *GetProtocolRoute() const;
    [[nodiscard]] GlobalQueue *GetGlobalQueue() const;

    ISubSystem *GetSystemByName(std::string_view sys) const;

    const YAML::Node &GetServerConfig();
    int32_t GetServerID();

private:
    bool LoadServerDLL(const std::string &path);
    awaitable<void> WaitForConnect();

    template<SystemType T>
    T *CreateSystem(const int priority = 0) {
        if (bInited)
            return nullptr;

        const auto res = new T(this);

        system_map_.insert_or_assign(typeid(T), res);
        name_to_system_.insert_or_assign(res->GetSystemName(), res);

        init_priority_.push({ priority, typeid(T) });
        dest_priority_.push({ priority, typeid(T) });

        return res;
    }
};
