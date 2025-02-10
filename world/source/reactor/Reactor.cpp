#include "../../include/reactor/Reactor.h"
#include "../../include/reactor/TaskQueue.h"


UReactor::~UReactor() {
    if (queue_ != nullptr)
        queue_->OnReactorRelease();
}

void UReactor::SetTaskQueue(const std::shared_ptr<UTaskQueue> &queue) {
    queue_ = queue;
}

std::shared_ptr<UTaskQueue> UReactor::GetTaskQueue() const {
    return queue_;
}

void UReactor::PushTask(const std::function<void(UReactor *)> &task) const {
    if (queue_ != nullptr)
        queue_->PushTask(task);
}

void UReactor::PushTask(std::function<void(UReactor *)> &&task) const {
    if (queue_ != nullptr)
        queue_->PushTask(std::move(task));
}
