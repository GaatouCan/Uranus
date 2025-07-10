#pragma once

#include "Module.h"
#include "ConcurrentDeque.h"
#include "utils.h"

#include <memory>
#include <shared_mutex>
#include <absl/container/flat_hash_map.h>


struct FWatchdog;
class IPlugin;

using absl::flat_hash_map;


class BASE_API UMonitor final : public IModule {

    DECLARE_MODULE(UMonitor)

    struct FCommandNode {
        std::string sender;
        std::string type;
        std::string args;
        std::string comment;
        long long timestamp;
    };

    typedef IPlugin *(*APluginCreator)(UMonitor *);
    typedef void (*APluginDestroyer)(IPlugin *);

protected:
    explicit UMonitor(UServer *server);

    void Initial() override;
    void Start() override;
    void Stop() override;

public:
    ~UMonitor() override;

    constexpr const char *GetModuleName() const override {
        return "Monitor Module";
    }

    void OnCommand(const std::string &sender, const std::string &type, const std::string &args, const std::string &comments = "");

#ifdef URANUS_WATCHDOG
    void RegisterWatchdog(const std::weak_ptr<FWatchdog> &watchdog);

private:
    awaitable<void> WatchdogLooping();
#endif

private:
    awaitable<void> ScheduleCommand();
    void HandleCommand(const FCommandNode &node);

    void OnShutdownService(int32_t sid) const;

private:
    asio::io_context mIOContext;
    asio::executor_work_guard<asio::io_context::executor_type> mGuard;

    std::thread mThread;

#ifdef URANUS_WATCHDOG
    flat_hash_map<int32_t, std::weak_ptr<FWatchdog>> mServiceWatchdogMap;
    flat_hash_map<int64_t, std::weak_ptr<FWatchdog>> mAgentWatchdogMap;
    mutable std::shared_mutex mWatchdogMutex;

    ASystemTimer mLoopTimer;
#endif

    TConcurrentDeque<FCommandNode> mCommandQueue;

    flat_hash_map<std::string, IPlugin *> mPluginMap;

    std::atomic_bool bHandling;

    constexpr static int LOOPING_RATE = 30;
};

