#pragma once

#include "sub_system.h"
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
    ATcpAcceptor acceptor_;

    AModuleHandle        module_;
    IServerLogic *      server_;
    AServerDestroyer     destroyer_;

    class UConfigManager *       cfg_mgr_;
    class ULoginAuthenticator *  login_authenticator_;
    class USceneManager *        scene_mgr_;
    class UGlobalQueue *         global_queue_;
    class UProtocolRoute *       proto_route_;

    std::unordered_map<std::string, AConnectionPointer, FStringViewHash, FStringViewEqual> conn_map_;

    ASystemTimer full_timer_;

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
    std::unordered_map<std::string, ISubSystem *, FStringViewHash, FStringViewEqual>  name_to_sys_;

    AThreadID tid_;

    bool                inited_;
    std::atomic_bool    running_;

public:
    UGameWorld();
    ~UGameWorld();

    DISABLE_COPY_MOVE(UGameWorld)

    UGameWorld &Init(const std::string &path);
    UGameWorld &Run();
    UGameWorld &Shutdown();

    void ForceDisconnectAll();

    void RemoveConnection(const std::string &key);
    void RemoveConnection(std::string_view key);

    asio::io_context &GetIOContext();

    AThreadID GetThreadID() const;
    [[nodiscard]] bool IsMainThread() const;

    template<SYSTEM_TYPE T>
    T *GetSystem() const noexcept {
        if (const auto iter = sys_map_.find(typeid(T)); iter != sys_map_.end())
            return dynamic_cast<T *>(iter->second);
        return nullptr;
    }

    [[nodiscard]] UConfigManager *       GetConfigManager() const;
    [[nodiscard]] USceneManager *        GetSceneManager() const;
    [[nodiscard]] ULoginAuthenticator *  GetLoginAuthenticator() const;
    [[nodiscard]] UProtocolRoute *       GetProtocolRoute() const;
    [[nodiscard]] UGlobalQueue *         GetGlobalQueue() const;

    ISubSystem *GetSystemByName(std::string_view sys) const;

    const YAML::Node &GetServerConfig();
    int32_t GetServerID();

private:
    bool LoadServerDLL(const std::string &path);
    awaitable<void> WaitForConnect();

    template<SYSTEM_TYPE T>
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
