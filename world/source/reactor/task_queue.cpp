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
      in_global_(false),
      removed_(false) {
}

IReactor *UTaskQueue::GetReactor() const {
    return reactor_;
}

void UTaskQueue::PushTask(const AReactorTask &task) {
    if (running_) {
        std::unique_lock lock(mtx_);
        wait_queue_.push(task);
        return;
    }

    {
        std::unique_lock lock(mtx_);
        cur_queue_.push(task);
    }

    global_->OnPushTask(shared_from_this());
}

void UTaskQueue::PushTask(AReactorTask &&task) {
    if (running_) {
        std::unique_lock lock(mtx_);
        wait_queue_.emplace(std::move(task));
        return;
    }

    {
        std::unique_lock lock(mtx_);
        cur_queue_.emplace(std::move(task));
    }

    global_->OnPushTask(shared_from_this());
}

bool UTaskQueue::IsEmpty() const {
    std::shared_lock lock(mtx_);
    return running_ ? wait_queue_.empty() : cur_queue_.empty();
}

bool UTaskQueue::IsRunning() const {
    return running_;
}

bool UTaskQueue::IsRemoved() const {
    return removed_;
}

void UTaskQueue::OnPushToGlobal() {
    in_global_ = true;
}

void UTaskQueue::OnPopFromGlobal() {
    in_global_ = false;
}

bool UTaskQueue::IsInGlobal() const {
    return in_global_;
}

void UTaskQueue::HandleTask(const int rate) {
    int num = std::ceil(cur_queue_.size() * (rate / 10000));

    const auto maxHandleTime = NowTimePoint() + TASK_QUEUE_MAX_HANDLE_SECOND;

    while ((num-- > 0) && !removed_ && (NowTimePoint() <= maxHandleTime)) {
        if (removed_)
            break;

        auto val = cur_queue_.front();
        cur_queue_.pop();
        std::invoke(val, reactor_);
    }
}

void UTaskQueue::OnStart() {
    running_ = true;
}

void UTaskQueue::OnStop() {
    running_ = false;

    std::unique_lock lock(mtx_);
    while (!wait_queue_.empty()) {
        cur_queue_.emplace(std::move(wait_queue_.front()));
        wait_queue_.pop();
    }
}

void UTaskQueue::OnRemove() {
    removed_ = true;
}

void UTaskQueue::OnReactorRelease() {
    global_->RemoveReactor(reactor_);
    reactor_ = nullptr;
}
