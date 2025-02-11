#include "../../include/reactor/reactor.h"
#include "../../include/reactor/task_queue.h"


IReactor::~IReactor() {
    if (mQueue != nullptr)
        mQueue->OnReactorRelease();
}

void IReactor::SetTaskQueue(const std::shared_ptr<TaskQueue> &queue) {
    mQueue = queue;
}

std::shared_ptr<TaskQueue> IReactor::GetTaskQueue() const {
    return mQueue;
}

void IReactor::PushTask(const std::function<void(IReactor *)> &task) const {
    if (mQueue != nullptr)
        mQueue->PushTask(task);
}

void IReactor::PushTask(std::function<void(IReactor *)> &&task) const {
    if (mQueue != nullptr)
        mQueue->PushTask(std::move(task));
}
