#pragma once

#include "Module.h"
#include "ConcurrentDeque.h"
#include "Utils.h"

#include <memory>
#include <shared_mutex>
#include <absl/container/flat_hash_map.h>


struct FWatchdog;
class IPluginBase;

using absl::flat_hash_map;


class BASE_API UMonitor final : public IModuleBase {

    DECLARE_MODULE(UMonitor)

    struct FCommandNode {
        std::string sender;
        std::string type;
        std::string args;
        std::string comment;
        long long timestamp;
    };

    typedef IPluginBase *(*APluginCreator)(UMonitor *);
    typedef void (*APluginDestroyer)(IPluginBase *);

protected:
    UMonitor();

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

    ASteadyTimer mLoopTimer;
#endif

    TConcurrentDeque<FCommandNode> mCommandQueue;

    flat_hash_map<std::string, IPluginBase *> mPluginMap;

    std::atomic_bool bHandling;

    constexpr static int LOOPING_RATE = 30;
};

