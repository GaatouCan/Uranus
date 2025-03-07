#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#include "../common.h"


constexpr auto TASK_QUEUE_MAX_HANDLE_SECOND = std::chrono::seconds(3);


class UGlobalQueue;
class IReactor;
using AReactorTask = std::function<void(IReactor*)>;


class UTaskQueue final : public std::enable_shared_from_this<UTaskQueue> {

    UGlobalQueue *global_;
    IReactor *reactor_;

    std::queue<AReactorTask> cur_queue_;
    std::queue<AReactorTask> wait_queue_;

    mutable std::shared_mutex mtx_;

    std::atomic_bool running_;
    std::atomic_bool in_global_;
    std::atomic_bool removed_;

public:
    UTaskQueue() = delete;

    UTaskQueue(UGlobalQueue *global, IReactor *reactor);

    DISABLE_COPY_MOVE(UTaskQueue)

    [[nodiscard]] IReactor *GetReactor() const;

    void PushTask(const AReactorTask &task);
    void PushTask(AReactorTask &&task);

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
