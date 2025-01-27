#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <mutex>
#include <shared_mutex>

#include "../common.h"


class UGlobalQueue;
class UReactor;
using AReactorTask = std::function<void(UReactor*)>;

class UTaskQueue final : public std::enable_shared_from_this<UTaskQueue> {

    UGlobalQueue *mGlobal;
    UReactor *mReactor;

    std::queue<AReactorTask> mCurrentQueue;
    std::queue<AReactorTask> mWaitingQueue;

    std::mutex mMutex;
    mutable std::shared_mutex mSharedMutex;

    std::atomic_bool bRunning;
    std::atomic_bool bInGlobal;
    std::atomic_bool bRemoved;

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
