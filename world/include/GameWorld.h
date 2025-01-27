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

    asio::io_context mContext;
    ATcpAcceptor mAcceptor;

    AModuleHandle mModule;
    IServerLogic *mServer;
    AServerDestroyer mServerDestroyer;

    class UConfigManager *mConfigManager;
    class ULoginAuthenticator *mLoginAuthenticator;
    class USceneManager *mSceneManager;
    class UGlobalQueue *mGlobalQueue;
    class UProtocolRoute *mProtocolRoute;

    std::unordered_map<std::string, AConnectionPointer, FStringViewHash, FStringViewEqual>
    mConnectionMap;

    ASystemTimer mFullTimer;

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

    std::priority_queue<FSystemPriority, std::vector<FSystemPriority>, std::greater<> > mInitPriority;
    std::priority_queue<FSystemPriority, std::vector<FSystemPriority>, std::less<> > mDestPriority;

    std::unordered_map<std::type_index, ISubSystem *> mSystemMap;
    std::unordered_map<std::string, ISubSystem *, FStringViewHash, FStringViewEqual> mNameToSystem;

    // std::function<void(const AConnectionPointer &)> mConnectionFilter;

    AThreadID mThreadID;

    bool bInited;
    std::atomic_bool bRunning;

public:
    UGameWorld();
    ~UGameWorld();

    DISABLE_COPY_MOVE(UGameWorld)

    UGameWorld &Init(const std::string &dllPath);
    UGameWorld &Run();
    UGameWorld &Shutdown();

    // void FilterConnection(const std::function<void(const AConnectionPointer &)> &filter);

    void RemoveConnection(const std::string &key);
    void RemoveConnection(std::string_view key);

    asio::io_context &GetIOContext();

    AThreadID GetThreadID() const;
    [[nodiscard]] bool IsMainThread() const;

    template<SYSTEM_TYPE T>
    T *GetSystem() const noexcept {
        if (const auto iter = mSystemMap.find(typeid(T)); iter != mSystemMap.end())
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
        if (bInited)
            return nullptr;

        const auto res = new T(this);

        mSystemMap.insert_or_assign(typeid(T), res);
        mNameToSystem.insert_or_assign(res->GetSystemName(), res);

        mInitPriority.push({ priority, typeid(T) });
        mDestPriority.push({ priority, typeid(T) });

        return res;
    }
};
