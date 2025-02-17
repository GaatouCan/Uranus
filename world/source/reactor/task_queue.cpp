#include "../../include/reactor/task_queue.h"
#include "../../include/reactor/reactor.h"
#include "../../include/reactor/global_queue.h"

#ifdef __linux__
#include <cmath>
#endif


TaskQueue::TaskQueue(GlobalQueue *global, IReactor *reactor)
    : mGlobal(global),
      mReactor(reactor),
      bRunning(false),
      bInGlobal(false),
      bRemoved(false) {
}

IReactor *TaskQueue::GetReactor() const {
    return mReactor;
}

void TaskQueue::PushTask(const ReactorTask &task) {
    if (bRunning) {
        std::unique_lock lock(mMutex);
        mWaitQueue.push(task);
        return;
    }

    {
        std::unique_lock lock(mMutex);
        mCurQueue.push(task);
    }

    mGlobal->OnPushTask(shared_from_this());
}

void TaskQueue::PushTask(ReactorTask &&task) {
    if (bRunning) {
        std::unique_lock lock(mMutex);
        mWaitQueue.emplace(std::move(task));
        return;
    }

    {
        std::unique_lock lock(mMutex);
        mCurQueue.emplace(std::move(task));
    }

    mGlobal->OnPushTask(shared_from_this());
}

bool TaskQueue::IsEmpty() const {
    std::shared_lock lock(mMutex);
    return bRunning ? mWaitQueue.empty() : mCurQueue.empty();
}

bool TaskQueue::IsRunning() const {
    return bRunning;
}

bool TaskQueue::IsRemoved() const {
    return bRemoved;
}

void TaskQueue::OnPushToGlobal() {
    bInGlobal = true;
}

void TaskQueue::OnPopFromGlobal() {
    bInGlobal = false;
}

bool TaskQueue::IsInGlobal() const {
    return bInGlobal;
}

void TaskQueue::HandleTask(const int rate) {
    int num = std::ceil(mCurQueue.size() * (rate / 10000));

    while (num-- > 0 && !bRemoved) {
        if (bRemoved)
            break;

        auto val = mCurQueue.front();
        mCurQueue.pop();
        std::invoke(val, mReactor);
    }
}

void TaskQueue::OnStart() {
    bRunning = true;
}

void TaskQueue::OnStop() {
    bRunning = false;

    std::unique_lock lock(mMutex);
    while (!mWaitQueue.empty()) {
        mCurQueue.emplace(std::move(mWaitQueue.front()));
        mWaitQueue.pop();
    }
}

void TaskQueue::OnRemove() {
    bRemoved = true;
}

void TaskQueue::OnReactorRelease() {
    mGlobal->RemoveReactor(mReactor);
    mReactor = nullptr;
}
