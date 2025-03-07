#include "../../include/reactor/global_queue.h"
#include "../../include/game_world.h"
#include "../../include/reactor/reactor.h"
#include "../../include/reactor/task_queue.h"

#include <spdlog/spdlog.h>

UGlobalQueue::UGlobalQueue(UGameWorld *world)
    : world_(world) {
}

UGlobalQueue::~UGlobalQueue() {
    spdlog::info("{} - Shutdown.", __FUNCTION__);
    queue_.Quit();

    for (auto &th: worker_vec_) {
        if (th.joinable())
            th.join();
    }
}

void UGlobalQueue::Init() {
    const auto &cfg = world_->GetServerConfig();
    const int num = cfg["server"]["work_thread"].as<int>();

    for (int idx = 0; idx < num; idx++) {
        worker_vec_.emplace_back([this] {
            while (queue_.IsRunning()) {
                queue_.Wait();

                if (!queue_.IsRunning())
                    break;

                auto res = queue_.PopFront();
                if (!res.has_value()) continue;

                auto task_queue = res.value();
                if (task_queue == nullptr)
                    continue;

                if (task_queue->IsRemoved())
                    continue;

                task_queue->OnPopFromGlobal();
                task_queue->OnStart();
                task_queue->HandleTask(10000);

                if (task_queue->IsRemoved())
                    continue;

                task_queue->OnStop();

                if (task_queue->IsEmpty()) {
                    std::unique_lock lock(empty_mtx_);
                    empty_set_.emplace(task_queue);
                    continue;
                }

                queue_.PushBack(task_queue);
                task_queue->OnPushToGlobal();
            }
        });
    }

    spdlog::info("Global Queue Work With {} Thread(s).", num);
}

std::shared_ptr<UTaskQueue> UGlobalQueue::RegisterReactor(IReactor *reactor) {
    if (reactor == nullptr)
        return nullptr;

    if (const auto res = FindByReactor(reactor); res != nullptr)
        return res;

    auto queue = std::make_shared<UTaskQueue>(this, reactor);
    reactor->SetTaskQueue(queue);

    std::scoped_lock lock(reactor_mtx_, empty_mtx_);
    reactor_map_[reactor] = queue;
    empty_set_.emplace(queue->weak_from_this());

    return queue;
}

std::shared_ptr<UTaskQueue> UGlobalQueue::FindByReactor(const IReactor *reactor) const {
    if (reactor == nullptr)
        return nullptr;

    std::shared_lock lock(reactor_mtx_);
    if (const auto it = reactor_map_.find(const_cast<IReactor *>(reactor)); it != reactor_map_.end())
        return it->second.expired() ? nullptr : it->second.lock();

    return nullptr;
}

void UGlobalQueue::OnPushTask(const std::shared_ptr<UTaskQueue> &queue) {
    if (queue == nullptr)
        return;

    if (queue->IsEmpty())
        return;

    {
        std::shared_lock lock(empty_mtx_);
        if (!empty_set_.contains(queue->weak_from_this()))
            return;
    }

    {
        std::unique_lock lock(empty_mtx_);
        empty_set_.erase(queue->weak_from_this());
    }

    queue_.PushBack(queue);
    queue->OnPushToGlobal();
}

void UGlobalQueue::RemoveReactor(const IReactor *reactor) {
    const auto queue = FindByReactor(reactor);
    if (queue == nullptr)
        return;

    queue->OnRemove();

    std::scoped_lock lock(reactor_mtx_, empty_mtx_);

    reactor_map_.erase(const_cast<IReactor *>(reactor));
    empty_set_.erase(queue->weak_from_this());
}
