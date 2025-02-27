#include "../../include/reactor/reactor.h"
#include "../../include/reactor/task_queue.h"


IReactor::~IReactor() {
    if (queue_ != nullptr)
        queue_->OnReactorRelease();
}

void IReactor::SetTaskQueue(const std::shared_ptr<TaskQueue> &queue) {
    queue_ = queue;
}

std::shared_ptr<TaskQueue> IReactor::GetTaskQueue() const {
    return queue_;
}

void IReactor::PushTask(const std::function<void(IReactor *)> &task) const {
    if (queue_ != nullptr)
        queue_->PushTask(task);
}

void IReactor::PushTask(std::function<void(IReactor *)> &&task) const {
    if (queue_ != nullptr)
        queue_->PushTask(std::move(task));
}
