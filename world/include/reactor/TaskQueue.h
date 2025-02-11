#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <shared_mutex>

#include "../common.h"


class GlobalQueue;
class IReactor;
using ReactorTask = std::function<void(IReactor*)>;

class TaskQueue final : public std::enable_shared_from_this<TaskQueue> {

    GlobalQueue *global_;
    IReactor *reactor_;

    std::queue<ReactorTask> cur_queue_;
    std::queue<ReactorTask> wait_queue_;

    // std::mutex mMutex;
    mutable std::shared_mutex mutex_;

    std::atomic_bool running_;
    std::atomic_bool in_global_;
    std::atomic_bool removed_;

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
