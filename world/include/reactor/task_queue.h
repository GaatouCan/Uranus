#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#include "../common.h"


constexpr auto TASK_QUEUE_MAX_HANDLE_SECOND = std::chrono::seconds(3);


class GlobalQueue;
class IReactor;
using ReactorTask = std::function<void(IReactor*)>;

class TaskQueue final : public std::enable_shared_from_this<TaskQueue> {

    GlobalQueue *mGlobal;
    IReactor *mReactor;

    std::queue<ReactorTask> mCurQueue;
    std::queue<ReactorTask> mWaitQueue;

    // std::mutex mMutex;
    mutable std::shared_mutex mMutex;

    std::atomic_bool bRunning;
    std::atomic_bool bInGlobal;
    std::atomic_bool bRemoved;

public:
    TaskQueue() = delete;

    TaskQueue(GlobalQueue *global, IReactor *reactor);

    DISABLE_COPY_MOVE(TaskQueue)

    [[nodiscard]] IReactor *GetReactor() const;

    void PushTask(const ReactorTask &task);
    void PushTask(ReactorTask &&task);

    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] bool IsRunning() const;
    [[nodiscard]] bool IsRemoved() const;

    void OnPushToGlobal();
    void OnPopFromGlobal();

    [[nodiscard]] bool IsInGlobal() const;

    void HandleTask(int rate);

    void OnStart();
    void OnStop();
    void OnRemove();

    void OnReactorRelease();
};
