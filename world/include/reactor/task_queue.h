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

    std::queue<AReactorTask> curQueue_;
    std::queue<AReactorTask> waitQueue_;

    mutable std::shared_mutex mutex_;

    std::atomic_bool running_;
    std::atomic_bool inGlobal_;
    std::atomic_bool removed_;

public:
    UTaskQueue() = delete;

    UTaskQueue(UGlobalQueue *global, IReactor *reactor);

    DISABLE_COPY_MOVE(UTaskQueue)

    [[nodiscard]] IReactor *getReactor() const;

    void pushTask(const AReactorTask &task);
    void pushTask(AReactorTask &&task);

    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool running() const;
    [[nodiscard]] bool removed() const;

    void onPushToGlobal();
    void onPopFromGlobal();

    [[nodiscard]] bool isInGlobal() const;

    void handleTask(int rate);

    void onStart();
    void onStop();
    void onRemove();

    void onReactorRelease();
};
