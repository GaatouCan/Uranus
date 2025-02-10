#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <shared_mutex>

#include "../common.h"


class UGlobalQueue;
class UReactor;
using AReactorTask = std::function<void(UReactor*)>;

class UTaskQueue final : public std::enable_shared_from_this<UTaskQueue> {

    UGlobalQueue *global_;
    UReactor *reactor_;

    std::queue<AReactorTask> curQueue_;
    std::queue<AReactorTask> waitQueue_;

    // std::mutex mMutex;
    mutable std::shared_mutex mutex_;

    std::atomic_bool running_;
    std::atomic_bool inGlobal_;
    std::atomic_bool removed_;

public:
    UTaskQueue() = delete;

    UTaskQueue(UGlobalQueue *global, UReactor *reactor);

    DISABLE_COPY_MOVE(UTaskQueue)

    [[nodiscard]] UReactor *GetReactor() const;

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
