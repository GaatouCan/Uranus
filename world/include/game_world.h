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

    asio::io_context ctx_;
    TcpAcceptor acceptor_;

    ModuleHandle        module_;
    IServerLogic *      server_;
    ServerDestroyer     destroyer_;

    class ConfigManager *       cfg_mgr_;
    class LoginAuthenticator *  login_authenticator_;
    class SceneManager *        scene_mgr_;
    class GlobalQueue *         global_queue_;
    class ProtocolRoute *       proto_route_;

    std::unordered_map<std::string, ConnectionPointer, StringViewHash, StringViewEqual> conn_map_;

    SystemTimer full_timer_;

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

    std::priority_queue<SystemPriority, std::vector<SystemPriority>, std::greater<>>    init_priority_;
    std::priority_queue<SystemPriority, std::vector<SystemPriority>, std::less<>>       dest_priority_;

    std::unordered_map<std::type_index, ISubSystem *>                               sys_map_;
    std::unordered_map<std::string, ISubSystem *, StringViewHash, StringViewEqual>  name_to_sys_;

    ThreadID tid_;

    bool                inited_;
    std::atomic_bool    running_;

public:
    GameWorld();
    ~GameWorld();

    DISABLE_COPY_MOVE(GameWorld)

    GameWorld &Init(const std::string &path);
    GameWorld &Run();
    GameWorld &Shutdown();

    void ForceDisconnectAll();

    void RemoveConnection(const std::string &key);
    void RemoveConnection(std::string_view key);

    asio::io_context &GetIOContext();

    ThreadID GetThreadID() const;
    [[nodiscard]] bool IsMainThread() const;

    template<SystemType T>
    T *GetSystem() const noexcept {
        if (const auto iter = sys_map_.find(typeid(T)); iter != sys_map_.end())
            return dynamic_cast<T *>(iter->second);
        return nullptr;
    }

    [[nodiscard]] ConfigManager *       GetConfigManager() const;
    [[nodiscard]] SceneManager *        GetSceneManager() const;
    [[nodiscard]] LoginAuthenticator *  GetLoginAuthenticator() const;
    [[nodiscard]] ProtocolRoute *       GetProtocolRoute() const;
    [[nodiscard]] GlobalQueue *         GetGlobalQueue() const;

    ISubSystem *GetSystemByName(std::string_view sys) const;

    const YAML::Node &GetServerConfig();
    int32_t GetServerID();

private:
    bool LoadServerDLL(const std::string &path);
    awaitable<void> WaitForConnect();

    template<SystemType T>
    T *CreateSystem(const int priority = 0) {
        if (inited_)
            return nullptr;

        const auto res = new T(this);

        sys_map_.insert_or_assign(typeid(T), res);
        name_to_sys_.insert_or_assign(res->GetSystemName(), res);

        init_priority_.push({ priority, typeid(T) });
        dest_priority_.push({ priority, typeid(T) });

        return res;
    }
};
