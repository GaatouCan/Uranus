#include "../../include/reactor/task_queue.h"

#include "utils.h"
#include "../../include/reactor/reactor.h"
#include "../../include/reactor/global_queue.h"

#ifdef __linux__
#include <cmath>
#endif


UTaskQueue::UTaskQueue(UGlobalQueue *global, IReactor *reactor)
    : global_(global),
      reactor_(reactor),
      running_(false),
      inGlobal_(false),
      removed_(false) {
}

IReactor *UTaskQueue::getReactor() const {
    return reactor_;
}

void UTaskQueue::pushTask(const AReactorTask &task) {
    if (running_) {
        std::unique_lock lock(mutex_);
        waitQueue_.push(task);
        return;
    }

    {
        std::unique_lock lock(mutex_);
        curQueue_.push(task);
    }

    global_->onPushTask(shared_from_this());
}

void UTaskQueue::pushTask(AReactorTask &&task) {
    if (running_) {
        std::unique_lock lock(mutex_);
        waitQueue_.emplace(std::move(task));
        return;
    }

    {
        std::unique_lock lock(mutex_);
        curQueue_.emplace(std::move(task));
    }

    global_->onPushTask(shared_from_this());
}

bool UTaskQueue::empty() const {
    std::shared_lock lock(mutex_);
    return running_ ? waitQueue_.empty() : curQueue_.empty();
}

bool UTaskQueue::running() const {
    return running_;
}

bool UTaskQueue::removed() const {
    return removed_;
}

void UTaskQueue::onPushToGlobal() {
    inGlobal_ = true;
}

void UTaskQueue::onPopFromGlobal() {
    inGlobal_ = false;
}

bool UTaskQueue::isInGlobal() const {
    return inGlobal_;
}

void UTaskQueue::handleTask(const int rate) {
    int num = std::ceil(curQueue_.size() * (rate / 10000));

    const auto maxHandleTime = NowTimePoint() + TASK_QUEUE_MAX_HANDLE_SECOND;

    while ((num-- > 0) && !removed_ && (NowTimePoint() <= maxHandleTime)) {
        if (removed_)
            break;

        auto val = curQueue_.front();
        curQueue_.pop();
        std::invoke(val, reactor_);
    }
}

void UTaskQueue::onStart() {
    running_ = true;
}

void UTaskQueue::onStop() {
    running_ = false;

    std::unique_lock lock(mutex_);
    while (!waitQueue_.empty()) {
        curQueue_.emplace(std::move(waitQueue_.front()));
        waitQueue_.pop();
    }
}

void UTaskQueue::onRemove() {
    removed_ = true;
}

void UTaskQueue::onReactorRelease() {
    global_->removeReactor(reactor_);
    reactor_ = nullptr;
}
