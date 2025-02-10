#include "../../include/reactor/TaskQueue.h"
#include "../../include/reactor/Reactor.h"
#include "../../include/reactor/GlobalQueue.h"

UTaskQueue::UTaskQueue(UGlobalQueue *global, UReactor *reactor)
    : global_(global),
      reactor_(reactor),
      running_(false),
      inGlobal_(false),
      removed_(false) {
}

UReactor *UTaskQueue::GetReactor() const {
    return reactor_;
}

void UTaskQueue::PushTask(const AReactorTask &task) {
    if (running_) {
        std::unique_lock lock(mutex_);
        waitQueue_.push(task);
        return;
    }

    {
        std::unique_lock lock(mutex_);
        curQueue_.push(task);
    }

    global_->OnPushTask(shared_from_this());
}

void UTaskQueue::PushTask(AReactorTask &&task) {
    if (running_) {
        std::unique_lock lock(mutex_);
        waitQueue_.emplace(std::move(task));
        return;
    }

    {
        std::unique_lock lock(mutex_);
        curQueue_.emplace(std::move(task));
    }

    global_->OnPushTask(shared_from_this());
}

bool UTaskQueue::IsEmpty() const {
    std::shared_lock lock(mutex_);
    return running_ ? waitQueue_.empty() : curQueue_.empty();
}

bool UTaskQueue::IsRunning() const {
    return running_;
}

bool UTaskQueue::IsRemoved() const {
    return removed_;
}

void UTaskQueue::OnPushToGlobal() {
    inGlobal_ = true;
}

void UTaskQueue::OnPopFromGlobal() {
    inGlobal_ = false;
}

bool UTaskQueue::IsInGlobal() const {
    return inGlobal_;
}

void UTaskQueue::HandleTask(const int rate) {
    int num = std::ceil(curQueue_.size() * (rate / 10000));

    while (num-- > 0 && !removed_) {
        if (removed_)
            break;

        auto val = curQueue_.front();
        curQueue_.pop();
        std::invoke(val, reactor_);
    }
}

void UTaskQueue::OnStart() {
    running_ = true;
}

void UTaskQueue::OnStop() {
    running_ = false;

    std::unique_lock lock(mutex_);
    while (!waitQueue_.empty()) {
        curQueue_.emplace(std::move(waitQueue_.front()));
        waitQueue_.pop();
    }
}

void UTaskQueue::OnRemove() {
    removed_ = true;
}

void UTaskQueue::OnReactorRelease() {
    global_->RemoveReactor(reactor_);
    reactor_ = nullptr;
}
