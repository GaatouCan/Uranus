#include "../../include/reactor/TaskQueue.h"
#include "../../include/reactor/Reactor.h"
#include "../../include/reactor/GlobalQueue.h"

UTaskQueue::UTaskQueue(UGlobalQueue *global, UReactor *reactor)
    : mGlobal(global),
      mReactor(reactor),
      bRunning(false),
      bInGlobal(false),
      bRemoved(false) {
}

UReactor *UTaskQueue::GetReactor() const {
    return mReactor;
}

void UTaskQueue::PushTask(const AReactorTask &task) {
    if (bRunning) {
        std::scoped_lock lock(mMutex);
        mWaitingQueue.push(task);
        return;
    }

    {
        std::scoped_lock lock(mMutex);
        mCurrentQueue.push(task);
    }

    mGlobal->OnPushTask(shared_from_this());
}

void UTaskQueue::PushTask(AReactorTask &&task) {
    if (bRunning) {
        std::scoped_lock lock(mMutex);
        mWaitingQueue.emplace(std::move(task));
        return;
    }

    {
        std::scoped_lock lock(mMutex);
        mCurrentQueue.emplace(std::move(task));
    }

    mGlobal->OnPushTask(shared_from_this());
}

bool UTaskQueue::IsEmpty() const {
    std::shared_lock lock(mSharedMutex);
    return bRunning ? mWaitingQueue.empty() : mCurrentQueue.empty();
}

bool UTaskQueue::IsRunning() const {
    return bRunning;
}

bool UTaskQueue::IsRemoved() const {
    return bRemoved;
}

void UTaskQueue::OnPushToGlobal() {
    bInGlobal = true;
}

void UTaskQueue::OnPopFromGlobal() {
    bInGlobal = false;
}

bool UTaskQueue::IsInGlobal() const {
    return bInGlobal;
}

void UTaskQueue::HandleTask(const int rate) {
    int num = std::ceil(mCurrentQueue.size() * (rate / 10000));

    while (num-- > 0 && !bRemoved) {
        if (bRemoved)
            break;

        auto val = mCurrentQueue.front();
        mCurrentQueue.pop();
        std::invoke(val, mReactor);
    }
}

void UTaskQueue::OnStart() {
    bRunning = true;
}

void UTaskQueue::OnStop() {
    bRunning = false;

    std::scoped_lock lock(mMutex);
    while (!mWaitingQueue.empty()) {
        mCurrentQueue.emplace(std::move(mWaitingQueue.front()));
        mWaitingQueue.pop();
    }
}

void UTaskQueue::OnRemove() {
    bRemoved = true;
}

void UTaskQueue::OnReactorRelease() {
    mGlobal->RemoveReactor(mReactor);
    mReactor = nullptr;
}
