#include "../../include/reactor/reactor.h"
#include "../../include/reactor/task_queue.h"


IReactor::~IReactor() {
    if (queue_ != nullptr)
        queue_->onReactorRelease();
}

void IReactor::setTaskQueue(const std::shared_ptr<UTaskQueue> &queue) {
    queue_ = queue;
}

std::shared_ptr<UTaskQueue> IReactor::getTaskQueue() const {
    return queue_;
}

void IReactor::pushTask(const std::function<void(IReactor *)> &task) const {
    if (queue_ != nullptr)
        queue_->pushTask(task);
}

void IReactor::pushTask(std::function<void(IReactor *)> &&task) const {
    if (queue_ != nullptr)
        queue_->pushTask(std::move(task));
}
