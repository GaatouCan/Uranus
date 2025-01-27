#include "../../include/reactor/Reactor.h"
#include "../../include/reactor/TaskQueue.h"

UReactor::UReactor() {
}

UReactor::~UReactor() {
    if (mTaskQueue != nullptr)
        mTaskQueue->OnReactorRelease();
}

void UReactor::SetTaskQueue(const std::shared_ptr<UTaskQueue> &queue) {
    mTaskQueue = queue;
}

std::shared_ptr<UTaskQueue> UReactor::GetTaskQueue() const {
    return mTaskQueue;
}

void UReactor::PushTask(const std::function<void(UReactor *)> &task) const {
    if (mTaskQueue != nullptr)
        mTaskQueue->PushTask(task);
}

void UReactor::PushTask(std::function<void(UReactor *)> &&task) const {
    if (mTaskQueue != nullptr)
        mTaskQueue->PushTask(std::move(task));
}
