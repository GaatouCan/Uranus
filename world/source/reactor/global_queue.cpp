#include "../../include/reactor/GlobalQueue.h"
#include "../../include/GameWorld.h"
#include "../../include/reactor/Reactor.h"
#include "../../include/reactor/TaskQueue.h"

#include <spdlog/spdlog.h>

GlobalQueue::GlobalQueue(GameWorld *world)
    : world_(world) {
}

GlobalQueue::~GlobalQueue() {
    queue_.Quit();

    for (auto &th: worker_vec_) {
        if (th.joinable())
            th.join();
    }
}

void GlobalQueue::Init() {
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

                auto reactor = res.value();
                if (reactor == nullptr)
                    continue;

                if (reactor->IsRemoved())
                    continue;

                reactor->OnPopFromGlobal();
                reactor->OnStart();
                reactor->HandleTask(10000);

                if (reactor->IsRemoved())
                    continue;

                reactor->OnStop();

                if (reactor->IsEmpty()) {
                    std::unique_lock lock(empty_mutex_);
                    empty_set_.emplace(reactor);
                    continue;
                }

                queue_.PushBack(reactor);
                reactor->OnPushToGlobal();
            }
        });
    }

    spdlog::info("Reactor System Work With {} Thread(s).", num);
}

std::shared_ptr<TaskQueue> GlobalQueue::RegisterReactor(IReactor *reactor) {
    if (reactor == nullptr)
        return nullptr;

    if (const auto res = FindByReactor(reactor); res != nullptr)
        return res;

    auto queue = std::make_shared<TaskQueue>(this, reactor);
    reactor->SetTaskQueue(queue);

    std::scoped_lock lock(reactor_mutex_, empty_mutex_);
    reactor_map_[reactor] = queue;
    empty_set_.emplace(queue->weak_from_this());

    return queue;
}

std::shared_ptr<TaskQueue> GlobalQueue::FindByReactor(const IReactor *reactor) const {
    if (reactor == nullptr)
        return nullptr;

    std::shared_lock lock(reactor_mutex_);
    if (const auto it = reactor_map_.find(const_cast<IReactor *>(reactor)); it != reactor_map_.end())
        return it->second.expired() ? nullptr : it->second.lock();

    return nullptr;
}

void GlobalQueue::OnPushTask(const std::shared_ptr<TaskQueue> &queue) {
    if (queue == nullptr)
        return;

    if (queue->IsEmpty())
        return;

    {
        std::shared_lock lock(empty_mutex_);
        if (!empty_set_.contains(queue->weak_from_this()))
            return;
    }

    {
        std::unique_lock lock(empty_mutex_);
        empty_set_.erase(queue->weak_from_this());
    }

    queue_.PushBack(queue);
    queue->OnPushToGlobal();
}

void GlobalQueue::RemoveReactor(const IReactor *reactor) {
    const auto queue = FindByReactor(reactor);
    if (queue == nullptr)
        return;

    queue->OnRemove();

    std::scoped_lock lock(reactor_mutex_, empty_mutex_);

    reactor_map_.erase(const_cast<IReactor *>(reactor));
    empty_set_.erase(queue->weak_from_this());
}
