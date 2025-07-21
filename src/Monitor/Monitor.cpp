#include "Monitor.h"
#include "Watchdog.h"
#include "Argument.h"
#include "Server.h"
#include "Service/ServiceModule.h"

#include <spdlog/spdlog.h>


UMonitor::UMonitor()
    : mGuard(asio::make_work_guard(mIOContext)),
#ifdef URANUS_WATCHDOG
      mLoopTimer(mIOContext),
#endif
      bHandling(false) {
}

UMonitor::~UMonitor() {
    if (mThread.joinable()) {
        mThread.join();
    }

    Stop();
}

void UMonitor::Initial() {
    if (state_ != EModuleState::CREATED)
        return;

    mThread = std::thread([this] {
        SPDLOG_INFO("Monitor Run In Thread[{}]", utils::ThreadIDToInt(std::this_thread::get_id()));

        asio::signal_set signals(mIOContext, SIGINT, SIGTERM);
        signals.async_wait([this](auto, auto) {
            Stop();
        });
        mIOContext.run();
    });

    state_ = EModuleState::INITIALIZED;
}

void UMonitor::Start() {
    if (state_ != EModuleState::INITIALIZED)
        return;

#ifdef URANUS_WATCHDOG
    co_spawn(mIOContext, WatchdogLooping(), detached);
#endif

    state_ = EModuleState::RUNNING;
}

void UMonitor::Stop() {
    if (state_ == EModuleState::STOPPED)
        return;

    state_ = EModuleState::STOPPED;

    mGuard.reset();

#ifdef URANUS_WATCHDOG
    mLoopTimer.cancel();
#endif

    if (!mIOContext.stopped()) {
        mIOContext.stop();
    }
}

void UMonitor::OnCommand(const std::string &sender, const std::string &type, const std::string &args, const std::string &comments) {
    const FCommandNode node{
        std::string(sender),
        std::string(type),
        std::string(args),
        std::string(comments),
        utils::UnixTime()
    };

    const auto bEmpty = mCommandQueue.IsEmpty();
    mCommandQueue.PushBack(node);

    if (bEmpty && !bHandling && state_ == EModuleState::RUNNING) {
        bHandling = true;
        co_spawn(mIOContext, ScheduleCommand(), detached);
    }
}

#ifdef URANUS_WATCHDOG
void UMonitor::RegisterWatchdog(const std::weak_ptr<FWatchdog> &watchdog) {
    if (state_ != EModuleState::INITIALIZED || state_ != EModuleState::RUNNING)
        return;

    if (watchdog.expired())
        return;

    std::unique_lock lock(mWatchdogMutex);
    if (watchdog.lock()->sid > 0) {
        mServiceWatchdogMap[watchdog.lock()->sid] = watchdog;
    } else if (watchdog.lock()->pid > 0) {
        mAgentWatchdogMap[watchdog.lock()->pid] = watchdog;
    }
}

awaitable<void> UMonitor::WatchdogLooping() {
    SPDLOG_INFO("Begin Watchdog Looping...");

    auto handler = [](const std::shared_ptr<FWatchdog> &watchdog, const long long now) {
        if (watchdog == nullptr)
            return;

        if (watchdog->end <= 0)
            return;

        const std::string name = watchdog->sid > 0
                                     ? fmt::format("Service[{}]", watchdog->sid)
                                     : fmt::format("Agent[{}]", watchdog->pid);

        // 处理单个任务节点时间超过5秒
        if (watchdog->handle > 0 && (now - watchdog->handle) > 3) {
            const auto index = watchdog->total - watchdog->rest;
            SPDLOG_WARN("Watchdog Warning - {} Single Node[{}/{}] Handle Too Long",
                name, index, watchdog->total.load());
        }

        // 处理总时间过长
        if (watchdog->begin > 0) {
            //  已经处理完成了
            if (watchdog->end > 0 && watchdog->end - watchdog->begin >= 0) {
                if ((watchdog->end - watchdog->begin) > 10) {
                    const auto seconds = watchdog->end - watchdog->begin;

                    // 置0 避免多次警告
                    watchdog->begin = 0;
                    watchdog->end = 0;

                    SPDLOG_WARN("Watchdog Warning - {} Schedule Too Long, Use {} Second", name, seconds);
                }
            } else if (now - watchdog->begin > 10) {
                // 还没处理完当前队列的
                const auto seconds = now - watchdog->begin;
                const auto index = watchdog->total - watchdog->rest;
                SPDLOG_WARN("Watchdog Warning - {} Schedule[{}/{}] Too Long, Use {} Second",
                            name, index, watchdog->total.load(), seconds);
            }
        }

        // 队伍长度过长
        if (watchdog->total > 0 && watchdog->total > 1000) {
            SPDLOG_WARN("Watchdog Warning - {} Schedule Queue Too Long {}", name, watchdog->total.load());
        }

        // 过了比较久没动静就清掉
        if (watchdog->handle == 0 && watchdog->end > 0 && (now - watchdog->end) > 10) {
            watchdog->Reset();
        }
    };

    try {
        while (state_ == EModuleState::RUNNING) {
            mLoopTimer.expires_after(std::chrono::seconds(LOOPING_RATE));
            if (const auto [ec] = co_await mLoopTimer.async_wait(); ec) {
                SPDLOG_DEBUG("{:<20} - Timer Cancel, Looping Exit.", __FUNCTION__);
                co_return;
            }

            const auto now = utils::UnixTime();
            SPDLOG_DEBUG("{:<20} - Do Watchdog Scanning.", __FUNCTION__);

            std::shared_lock lock(mWatchdogMutex);

            for (auto it = mServiceWatchdogMap.begin(); it != mServiceWatchdogMap.end();) {
                if (const auto watchdog = it->second.lock(); watchdog) {
                    handler(watchdog, now);
                    ++it;
                } else {
                    mServiceWatchdogMap.erase(it++);
                }
            }

            for (auto it = mAgentWatchdogMap.begin(); it != mAgentWatchdogMap.end();) {
                if (const auto watchdog = it->second.lock(); watchdog) {
                    handler(watchdog, now);
                    ++it;
                } else {
                    mAgentWatchdogMap.erase(it++);
                }
            }
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, e.what());
    }
}
#endif

awaitable<void> UMonitor::ScheduleCommand() {
    if (state_ != EModuleState::RUNNING)
        co_return;

    std::queue<FCommandNode> queue;
    mCommandQueue.SwapTo(queue);

    while (!queue.empty() && state_ == EModuleState::RUNNING) {
        auto node = queue.front();
        queue.pop();

        if (state_ == EModuleState::RUNNING) {
            HandleCommand(node);
        }
    }

    if (!mCommandQueue.IsEmpty() && state_ == EModuleState::RUNNING) {
        co_spawn(mIOContext, ScheduleCommand(), detached);
    } else {
        bHandling = false;
    }
}

void UMonitor::HandleCommand(const FCommandNode &node) {
    FArgument args(node.args);

    if (node.type == "SHUTDOWN") {
        const auto sid = args.ReadInt();
        OnShutdownService(sid);
    }
}

void UMonitor::OnShutdownService(const int32_t sid) const {
    if (GetState() != EModuleState::RUNNING)
        return;

    if (auto *service = GetServer()->GetModule<UServiceModule>()) {
        service->ShutdownService(sid);
    }
}
