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

    asio::io_context context_;
    ATcpAcceptor acceptor_;

    AModuleHandle module_;
    IServerLogic *server_;
    AServerDestroyer destroyer_;

    class UConfigManager *configManager_;
    class ULoginAuthenticator *loginAuthenticator_;
    class USceneManager *sceneManager_;
    class UGlobalQueue *globalQueue_;
    class UProtoRoute *protoRoute_;

    std::unordered_map<std::string, AConnectionPointer, FStringViewHash, FStringViewEqual> connectionMap_;

    ASystemTimer fullTimer_;

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

    std::priority_queue<SystemPriority, std::vector<SystemPriority>, std::greater<>> initPriority_;
    std::priority_queue<SystemPriority, std::vector<SystemPriority>, std::less<>> destroyPriority_;

    std::unordered_map<std::type_index, ISubSystem *> systemMap_;
    std::unordered_map<std::string, ISubSystem *, FStringViewHash, FStringViewEqual> nameToSystem_;

    AThreadID tid_;

    bool inited_;
    std::atomic_bool running_;

public:
    UGameWorld();
    ~UGameWorld();

    DISABLE_COPY_MOVE(UGameWorld)

    UGameWorld &init(const std::string &path);
    UGameWorld &run();
    UGameWorld &shutdown();

    void forceDisconnectAll();

    void removeConnection(const std::string &key);
    void removeConnection(std::string_view key);

    asio::io_context &getIOContext();

    AThreadID getThreadID() const;
    [[nodiscard]] bool isMainThread() const;

    template<CSystemType T>
    T *getSystem() const noexcept {
        if (const auto iter = systemMap_.find(typeid(T)); iter != systemMap_.end())
            return dynamic_cast<T *>(iter->second);
        return nullptr;
    }

    [[nodiscard]] UConfigManager *getConfigManager() const;
    [[nodiscard]] USceneManager *getSceneManager() const;
    [[nodiscard]] ULoginAuthenticator *getLoginAuthenticator() const;
    [[nodiscard]] UProtoRoute *getProtoRoute() const;
    [[nodiscard]] UGlobalQueue *getGlobalQueue() const;

    ISubSystem *getSystemByName(std::string_view sys) const;

    const YAML::Node &getServerConfig();
    int32_t getServerID();

private:
    bool loadServerDLL(const std::string &path);
    awaitable<void> waitForConnect();

    template<CSystemType T>
    T *createSystem(const int priority = 0) {
        if (inited_)
            return nullptr;

        const auto res = new T(this);

        systemMap_.insert_or_assign(typeid(T), res);
        nameToSystem_.insert_or_assign(res->getSystemName(), res);

        initPriority_.push({ priority, typeid(T) });
        destroyPriority_.push({ priority, typeid(T) });

        return res;
    }
};
